// Copyright (C) 2026 Tamás Péter
// This file is part of SynapseEngine.
//
// SynapseEngine is free software: you can redistribute it and/or modify
// it under the terms of the GNU Affero General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// SynapseEngine is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License
// along with SynapseEngine. If not, see <https://www.gnu.org/licenses/>.

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