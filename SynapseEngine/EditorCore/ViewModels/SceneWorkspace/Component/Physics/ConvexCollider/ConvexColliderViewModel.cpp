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

#include "ConvexColliderViewModel.h"

namespace Syn
{
    ConvexColliderViewModel::ConvexColliderViewModel(ISelectionApi* selectionApi, IConvexColliderApi* colliderApi)
        : _selectionApi(selectionApi), _colliderApi(colliderApi)
    {}

    const ConvexColliderState& ConvexColliderViewModel::GetState() const
    {
        return _state;
    }

    void ConvexColliderViewModel::SyncWithEngine()
    {
        if (!_selectionApi || !_colliderApi)
            return;

        EntityID activeEntity = _selectionApi->GetSelectedEntity();

        if (activeEntity != NULL_ENTITY && _colliderApi->HasConvexCollider(activeEntity))
        {
            _state.hasComponent = true;

            _state.targetLodLevel = _colliderApi->GetConvexColliderTargetLodLevel(activeEntity);

            if (!_localOffsetDrag.IsDragging())
                _state.localOffset = _colliderApi->GetConvexColliderLocalOffset(activeEntity);
        }
        else
        {
            _state.hasComponent = false;
        }
    }

    void ConvexColliderViewModel::Dispatch(const ConvexColliderIntent& intent)
    {
        std::visit([this](auto&& arg)
            {
                using T = std::decay_t<decltype(arg)>;

                if constexpr (std::is_same_v<T, SetConvexColliderTargetLodLevelIntent>) HandleSetTargetLodLevel(arg);
                else if constexpr (std::is_same_v<T, SetConvexColliderLocalOffsetIntent>) HandleSetLocalOffset(arg); }, intent);
    }

    void ConvexColliderViewModel::HandleSetTargetLodLevel(const SetConvexColliderTargetLodLevelIntent& intent)
    {
        EntityID activeEntity = _selectionApi->GetSelectedEntity();
        if (activeEntity == NULL_ENTITY) return;

        _state.targetLodLevel = intent.targetLodLevel;
        _colliderApi->SetConvexColliderTargetLodLevel(activeEntity, intent.targetLodLevel);
    }

    void ConvexColliderViewModel::HandleSetLocalOffset(const SetConvexColliderLocalOffsetIntent& intent)
    {
        EntityID activeEntity = _selectionApi->GetSelectedEntity();
        if (activeEntity == NULL_ENTITY) return;

        _localOffsetDrag.Handle(intent.isDragging, intent.localOffset, _state.localOffset,
            [&](const glm::vec3& v) { _colliderApi->SetConvexColliderLocalOffset(activeEntity, v); },
            [&](const glm::vec3& s, const glm::vec3& e) { return std::make_shared<ChangeConvexColliderLocalOffsetCommand>(_colliderApi, activeEntity, s, e); });
    }
}