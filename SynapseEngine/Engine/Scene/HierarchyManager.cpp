#include "HierarchyManager.h"
#include <queue>
#include "Engine/Logger/SynLog.h"

namespace Syn
{
    HierarchyManager::HierarchyManager(Registry* registry)
        : _registry(registry)
    {}

    bool HierarchyManager::CanAttach(EntityID parent, EntityID child) const
    {
        if (parent == child) return false;

        auto hierarchyPool = _registry->GetPool<HierarchyComponent>();
        if (!hierarchyPool) return false;

        EntityID ancestor = parent;
        while (ancestor != NULL_ENTITY)
        {
            if (ancestor == child) return false;

            if (!hierarchyPool->Has(ancestor)) break;
            ancestor = hierarchyPool->Get(ancestor).parent;
        }

        return true;
    }

    void HierarchyManager::RebuildTopologicalArray()
    {
        std::vector<EntityID> newArray;
        std::vector<HierarchyLevelData> newLevels = _levels;

        uint32_t currentOffset = 0;
        auto hierarchyPool = _registry->GetPool<HierarchyComponent>();

        for (size_t i = 0; i < newLevels.size(); ++i)
        {
            auto& level = newLevels[i];

            // Allocate new capacity with 25% growth + 32 padding
            uint32_t newCapacity = level.activeCount + (level.activeCount / 4) + 32;

            level.startIndex = currentOffset;
            level.capacity = newCapacity;

            newArray.resize(currentOffset + newCapacity, NULL_ENTITY);

            // Copy existing active entities to the new tightly packed array
            const auto& oldLevel = _levels[i];
            for (uint32_t k = 0; k < oldLevel.activeCount; ++k)
            {
                EntityID entity = _topologicalArray[oldLevel.startIndex + k];
                newArray[currentOffset + k] = entity;

                // Update physical index in the component
                auto& comp = hierarchyPool->Get(entity);
                comp.topoIndex = currentOffset + k;
            }

            currentOffset += newCapacity;
        }

        _topologicalArray = std::move(newArray);
        _levels = std::move(newLevels);
    }

    void HierarchyManager::InsertIntoLevel(EntityID entity, uint32_t level)
    {
        // Ensure level data exists
        while (_levels.size() <= level) [[unlikely]] {
            _levels.push_back(HierarchyLevelData{ 0, 0, 0 });
        }

        // Trigger full rebuild if the current level's window is full
        if (_levels[level].activeCount >= _levels[level].capacity) [[unlikely]] {
            RebuildTopologicalArray();
        }

        auto& levelData = _levels[level];
        auto hierarchyPool = _registry->GetPool<HierarchyComponent>();
        auto& comp = hierarchyPool->Get(entity);

        // Append to the end of the active block
        uint32_t insertIdx = levelData.startIndex + levelData.activeCount;
        _topologicalArray[insertIdx] = entity;
        comp.topoIndex = insertIdx;
        comp.depthLevel = level;

        levelData.activeCount++;
    }

    void HierarchyManager::RemoveFromLevel(EntityID entity)
    {
        auto hierarchyPool = _registry->GetPool<HierarchyComponent>();

        if (!hierarchyPool->Has(entity)) [[unlikely]] return;

        auto& comp = hierarchyPool->Get(entity);
        if (comp.topoIndex == 0xFFFFFFFF) return; // Not in array

        auto& levelData = _levels[comp.depthLevel];

        // Find the last active entity in this level
        uint32_t lastActiveIdx = levelData.startIndex + levelData.activeCount - 1;
        EntityID lastEntity = _topologicalArray[lastActiveIdx];

        // O(1) Swap-and-Pop: move last entity to the removed entity's slot
        _topologicalArray[comp.topoIndex] = lastEntity;

        if (lastEntity != entity) {
            auto& lastEntityComp = hierarchyPool->Get(lastEntity);
            lastEntityComp.topoIndex = comp.topoIndex;
        }

        // Clear the popped slot and shrink active count
        _topologicalArray[lastActiveIdx] = NULL_ENTITY;
        levelData.activeCount--;
        comp.topoIndex = 0xFFFFFFFF;
    }

    void HierarchyManager::UpdateSubtreeLevels(EntityID root, int32_t levelDelta)
    {
        if (levelDelta == 0) return;

        auto hierarchyPool = _registry->GetPool<HierarchyComponent>();
        std::vector<EntityID> queue;

        // Push immediate children to the BFS queue
        auto& rootComp = hierarchyPool->Get(root);
        EntityID currChild = rootComp.firstChild;
        while (currChild != NULL_ENTITY) {
            queue.push_back(currChild);
            currChild = hierarchyPool->Get(currChild).nextSibling;
        }

        // BFS traversal to shift all descendants to their new levels
        size_t head = 0;
        while (head < queue.size())
        {
            EntityID entity = queue[head++];
            auto& comp = hierarchyPool->Get(entity);

            uint32_t newLevel = comp.depthLevel + levelDelta;

            // Shift to new topological level
            RemoveFromLevel(entity);
            InsertIntoLevel(entity, newLevel);

            // Queue next generation
            EntityID child = comp.firstChild;
            while (child != NULL_ENTITY) {
                queue.push_back(child);
                child = hierarchyPool->Get(child).nextSibling;
            }
        }
    }

    void HierarchyManager::AttachChild(EntityID parent, EntityID child)
    {
        if (!CanAttach(parent, child))
        {
            Warning("Hierarchy cycle prevented: Cannot attach entity {} to {}", child, parent);
            return;
        }

        auto hierarchyPool = _registry->GetPool<HierarchyComponent>();

        // Ensure both entities have the hierarchy component
        if (!hierarchyPool->Has(child)) [[unlikely]] hierarchyPool->Add(child);
        if (!hierarchyPool->Has(parent)) [[unlikely]] hierarchyPool->Add(parent);

        auto& parentComp = hierarchyPool->Get(parent);
        auto& childComp = hierarchyPool->Get(child);

        uint32_t oldLevel = childComp.depthLevel;
        uint32_t newLevel = parentComp.depthLevel + 1;

        // Cleanly unlink from previous parent without triggering DetachChild's level reset
        if (childComp.parent != NULL_ENTITY)
        {
            auto& oldParentComp = hierarchyPool->Get(childComp.parent);

            if (oldParentComp.firstChild == child)
                oldParentComp.firstChild = childComp.nextSibling;

            if (childComp.prevSibling != NULL_ENTITY)
                hierarchyPool->Get(childComp.prevSibling).nextSibling = childComp.nextSibling;

            if (childComp.nextSibling != NULL_ENTITY)
                hierarchyPool->Get(childComp.nextSibling).prevSibling = childComp.prevSibling;
        }

        // Link to the new parent (insert as first child)
        childComp.parent = parent;
        childComp.prevSibling = NULL_ENTITY;
        childComp.nextSibling = parentComp.firstChild;

        if (parentComp.firstChild != NULL_ENTITY) {
            hierarchyPool->Get(parentComp.firstChild).prevSibling = child;
        }
        parentComp.firstChild = child;

        // Shift the child and its entire subtree to the new depth
        RemoveFromLevel(child);
        InsertIntoLevel(child, newLevel);

        int32_t levelDelta = static_cast<int32_t>(newLevel) - static_cast<int32_t>(oldLevel);
        UpdateSubtreeLevels(child, levelDelta);

        _version++;
    }

    void HierarchyManager::DetachChild(EntityID child)
    {
        auto hierarchyPool = _registry->GetPool<HierarchyComponent>();
        if (!hierarchyPool->Has(child)) return;

        auto& childComp = hierarchyPool->Get(child);
        EntityID parent = childComp.parent;

        if (parent == NULL_ENTITY) return; // Already at root

        auto& parentComp = hierarchyPool->Get(parent);

        // Unlink from sibling chain
        if (parentComp.firstChild == child)
            parentComp.firstChild = childComp.nextSibling;

        if (childComp.prevSibling != NULL_ENTITY)
            hierarchyPool->Get(childComp.prevSibling).nextSibling = childComp.nextSibling;

        if (childComp.nextSibling != NULL_ENTITY)
            hierarchyPool->Get(childComp.nextSibling).prevSibling = childComp.prevSibling;

        // Clear local links
        childComp.parent = NULL_ENTITY;
        childComp.prevSibling = NULL_ENTITY;
        childComp.nextSibling = NULL_ENTITY;

        uint32_t oldLevel = childComp.depthLevel;
        uint32_t newLevel = 0; // Return to root level

        // Shift topological levels
        RemoveFromLevel(child);
        InsertIntoLevel(child, newLevel);

        int32_t levelDelta = static_cast<int32_t>(newLevel) - static_cast<int32_t>(oldLevel);
        UpdateSubtreeLevels(child, levelDelta);

        _version++;
    }

    std::span<const EntityID> HierarchyManager::GetEntitiesInLevel(uint32_t level) const
    {
        // Return a contiguous view of the level's memory block for taskflow processing
        if (level >= _levels.size()) return {};
        const auto& levelData = _levels[level];
        if (levelData.activeCount == 0) return {};

        return std::span<const EntityID>(&_topologicalArray[levelData.startIndex], levelData.activeCount);
    }

    void HierarchyManager::OnEntityCreated(EntityID entity)
    {
        auto hierarchyPool = _registry->GetPool<HierarchyComponent>();

        if (!hierarchyPool->Has(entity)) [[unlikely]] {
            hierarchyPool->Add(entity);
        }

        InsertIntoLevel(entity, 0);

		_version++;
    }

    void HierarchyManager::OnEntityDestroyed(EntityID entity)
    {
        auto hierarchyPool = _registry->GetPool<HierarchyComponent>();
        if (!hierarchyPool->Has(entity)) return;

        DetachChild(entity);

        auto& comp = hierarchyPool->Get(entity);
        EntityID currChild = comp.firstChild;

        while (currChild != NULL_ENTITY)
        {
            EntityID nextChild = hierarchyPool->Get(currChild).nextSibling;
            DetachChild(currChild);
            currChild = nextChild;
        }

        RemoveFromLevel(entity);

        _version++;
    }
}