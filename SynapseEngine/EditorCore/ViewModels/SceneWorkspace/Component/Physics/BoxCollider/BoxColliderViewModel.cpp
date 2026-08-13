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

#include "BoxColliderViewModel.h"

namespace Syn
{
    BoxColliderViewModel::BoxColliderViewModel(ISelectionApi* selectionApi, IBoxColliderApi* colliderApi)
        : _selectionApi(selectionApi), _colliderApi(colliderApi)
    {}

    const BoxColliderState& BoxColliderViewModel::GetState() const
    {
        return _state;
    }

    void BoxColliderViewModel::SyncWithEngine()
    {
        if (!_selectionApi || !_colliderApi)
            return;

        EntityID activeEntity = _selectionApi->GetSelectedEntity();

        if (activeEntity != NULL_ENTITY && _colliderApi->HasBoxCollider(activeEntity))
        {
            _state.hasComponent = true;

            if (!_halfExtentsDrag.IsDragging())
                _state.halfExtents = _colliderApi->GetBoxColliderHalfExtents(activeEntity);
            if (!_localOffsetDrag.IsDragging())
                _state.localOffset = _colliderApi->GetBoxColliderLocalOffset(activeEntity);
        }
        else
        {
            _state.hasComponent = false;
        }
    }

    void BoxColliderViewModel::Dispatch(const BoxColliderIntent& intent)
    {
        std::visit([this](auto&& arg)
            {
                using T = std::decay_t<decltype(arg)>;

                if constexpr (std::is_same_v<T, SetBoxColliderHalfExtentsIntent>) HandleSetHalfExtents(arg);
                else if constexpr (std::is_same_v<T, SetBoxColliderLocalOffsetIntent>) HandleSetLocalOffset(arg); }, intent);
    }

    void BoxColliderViewModel::HandleSetHalfExtents(const SetBoxColliderHalfExtentsIntent& intent)
    {
        EntityID activeEntity = _selectionApi->GetSelectedEntity();
        if (activeEntity == NULL_ENTITY) return;

        _halfExtentsDrag.Handle(intent.isDragging, intent.halfExtents, _state.halfExtents,
            [&](const glm::vec3& v) { _colliderApi->SetBoxColliderHalfExtents(activeEntity, v); },
            [&](const glm::vec3& s, const glm::vec3& e) { return std::make_shared<ChangeBoxColliderHalfExtentsCommand>(_colliderApi, activeEntity, s, e); });
    }

    void BoxColliderViewModel::HandleSetLocalOffset(const SetBoxColliderLocalOffsetIntent& intent)
    {
        EntityID activeEntity = _selectionApi->GetSelectedEntity();
        if (activeEntity == NULL_ENTITY) return;

        _localOffsetDrag.Handle(intent.isDragging, intent.localOffset, _state.localOffset,
            [&](const glm::vec3& v) { _colliderApi->SetBoxColliderLocalOffset(activeEntity, v); },
            [&](const glm::vec3& s, const glm::vec3& e) { return std::make_shared<ChangeBoxColliderLocalOffsetCommand>(_colliderApi, activeEntity, s, e); });
    }
}