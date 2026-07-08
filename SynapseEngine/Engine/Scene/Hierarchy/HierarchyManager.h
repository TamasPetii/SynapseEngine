#pragma once
#include "Engine/SynApi.h"
#include "Engine/Registry/Registry.h"
#include "Engine/Component/Core/HierarchyComponent.h"
#include "Engine/Registry/Type/TypeInfo.h"
#include "HierarchyWorkQueue.h"
#include "HierarchyLevelData.h"
#include <span>
#include <vector>
#include <unordered_map>
#include <memory>

namespace Syn
{
    class SYN_API HierarchyManager
    {
    public:
        HierarchyManager(Registry* registry);

        HierarchyManager(const HierarchyManager&) = delete;
        HierarchyManager& operator=(const HierarchyManager&) = delete;

        void AttachChild(EntityID parent, EntityID child);
        void DetachChild(EntityID child);
        bool CanAttach(EntityID parent, EntityID child) const;

        void OnEntityCreated(EntityID entity);
        void OnEntityDestroyed(EntityID entity);

        std::span<const EntityID> GetEntitiesInLevel(uint32_t level) const;
        uint32_t GetMaxActiveLevel() const { return static_cast<uint32_t>(_levels.size()); }
		const std::span<const HierarchyLevelData> GetLevels() const { return std::span<const HierarchyLevelData>(_levels.data(), _levels.size()); }
        uint64_t GetVersion() const { return _version; }

        template<typename TComponent>
        HierarchyWorkQueue* EnsureWorkQueue();
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

        std::unordered_map<TypeID, std::unique_ptr<HierarchyWorkQueue>> _workQueues;
    };

    template<typename TComponent>
    HierarchyWorkQueue* HierarchyManager::EnsureWorkQueue()
    {
        const TypeID id = TypeInfo<TComponent>::ID;
        auto it = _workQueues.find(id);

        [[unlikely]]
        if (it == _workQueues.end())
        {
            auto queue = std::make_unique<HierarchyWorkQueue>();
            auto* rawPtr = queue.get();
            _workQueues[id] = std::move(queue);
            return rawPtr;
        }

        return it->second.get();
    }
}