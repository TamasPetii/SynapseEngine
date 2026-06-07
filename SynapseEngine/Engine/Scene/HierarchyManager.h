#pragma once
#include "Engine/SynApi.h"
#include "Engine/Registry/Registry.h"
#include "Engine/Component/Core/HierarchyComponent.h"
#include <span>
#include <vector>

namespace Syn
{
    struct SYN_API HierarchyLevelData
    {
        uint32_t startIndex = 0;
        uint32_t capacity = 0;
        uint32_t activeCount = 0;
    };

    class SYN_API HierarchyManager
    {
    public:
        HierarchyManager(Registry* registry);

        void AttachChild(EntityID parent, EntityID child);
        void DetachChild(EntityID child);
        bool CanAttach(EntityID parent, EntityID child) const;

        void OnEntityCreated(EntityID entity);
        void OnEntityDestroyed(EntityID entity);

        std::span<const EntityID> GetEntitiesInLevel(uint32_t level) const;
        uint32_t GetMaxActiveLevel() const { return static_cast<uint32_t>(_levels.size()); }
		const std::span<const HierarchyLevelData> GetLevels() const { return std::span<const HierarchyLevelData>(_levels.data(), _levels.size()); }
        uint64_t GetVersion() const { return _version; }
    private:
        void InsertIntoLevel(EntityID entity, uint32_t level);
        void RemoveFromLevel(EntityID entity);
        void RebuildTopologicalArray();
        void UpdateSubtreeLevels(EntityID root, int32_t levelDelta);

    private:
        Registry* _registry;
        std::vector<EntityID> _topologicalArray;
        std::vector<HierarchyLevelData> _levels;
        uint64_t _version = 0;
    };
}