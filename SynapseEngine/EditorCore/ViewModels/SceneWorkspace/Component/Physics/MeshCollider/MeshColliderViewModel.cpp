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

#include "MeshColliderViewModel.h"

namespace Syn
{
    MeshColliderViewModel::MeshColliderViewModel(ISelectionApi* selectionApi, IMeshColliderApi* colliderApi)
        : _selectionApi(selectionApi), _colliderApi(colliderApi)
    {}

    const MeshColliderState& MeshColliderViewModel::GetState() const
    {
        return _state;
    }

    void MeshColliderViewModel::SyncWithEngine()
    {
        if (!_selectionApi || !_colliderApi)
            return;

        EntityID activeEntity = _selectionApi->GetSelectedEntity();

        if (activeEntity != NULL_ENTITY && _colliderApi->HasMeshCollider(activeEntity))
        {
            _state.hasComponent = true;

            _state.targetLodLevel = _colliderApi->GetMeshColliderTargetLodLevel(activeEntity);

            if (!_localOffsetDrag.IsDragging())
                _state.localOffset = _colliderApi->GetMeshColliderLocalOffset(activeEntity);
        }
        else
        {
            _state.hasComponent = false;
        }
    }

    void MeshColliderViewModel::Dispatch(const MeshColliderIntent& intent)
    {
        std::visit([this](auto&& arg)
            {
                using T = std::decay_t<decltype(arg)>;

                if constexpr (std::is_same_v<T, SetMeshColliderTargetLodLevelIntent>) HandleSetTargetLodLevel(arg);
                else if constexpr (std::is_same_v<T, SetMeshColliderLocalOffsetIntent>) HandleSetLocalOffset(arg); }, intent);
    }

    void MeshColliderViewModel::HandleSetTargetLodLevel(const SetMeshColliderTargetLodLevelIntent& intent)
    {
        EntityID activeEntity = _selectionApi->GetSelectedEntity();
        if (activeEntity == NULL_ENTITY) return;

        _state.targetLodLevel = intent.targetLodLevel;
        _colliderApi->SetMeshColliderTargetLodLevel(activeEntity, intent.targetLodLevel);
    }

    void MeshColliderViewModel::HandleSetLocalOffset(const SetMeshColliderLocalOffsetIntent& intent)
    {
        EntityID activeEntity = _selectionApi->GetSelectedEntity();
        if (activeEntity == NULL_ENTITY) return;

        _localOffsetDrag.Handle(intent.isDragging, intent.localOffset, _state.localOffset,
            [&](const glm::vec3& v) { _colliderApi->SetMeshColliderLocalOffset(activeEntity, v); },
            [&](const glm::vec3& s, const glm::vec3& e) { return std::make_shared<ChangeMeshColliderLocalOffsetCommand>(_colliderApi, activeEntity, s, e); });
    }
}