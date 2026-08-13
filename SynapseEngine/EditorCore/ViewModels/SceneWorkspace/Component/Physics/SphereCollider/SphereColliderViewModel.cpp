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

#include "SphereColliderViewModel.h"

namespace Syn
{
    SphereColliderViewModel::SphereColliderViewModel(ISelectionApi* selectionApi, ISphereColliderApi* colliderApi)
        : _selectionApi(selectionApi), _colliderApi(colliderApi)
    {}

    const SphereColliderState& SphereColliderViewModel::GetState() const
    {
        return _state;
    }

    void SphereColliderViewModel::SyncWithEngine()
    {
        if (!_selectionApi || !_colliderApi)
            return;

        EntityID activeEntity = _selectionApi->GetSelectedEntity();

        if (activeEntity != NULL_ENTITY && _colliderApi->HasSphereCollider(activeEntity))
        {
            _state.hasComponent = true;

            if (!_radiusDrag.IsDragging())
                _state.radius = _colliderApi->GetSphereColliderRadius(activeEntity);
            if (!_localOffsetDrag.IsDragging())
                _state.localOffset = _colliderApi->GetSphereColliderLocalOffset(activeEntity);
        }
        else
        {
            _state.hasComponent = false;
        }
    }

    void SphereColliderViewModel::Dispatch(const SphereColliderIntent& intent)
    {
        std::visit([this](auto&& arg)
            {
                using T = std::decay_t<decltype(arg)>;

                if constexpr (std::is_same_v<T, SetSphereColliderRadiusIntent>)      HandleSetRadius(arg);
                else if constexpr (std::is_same_v<T, SetSphereColliderLocalOffsetIntent>) HandleSetLocalOffset(arg); }, intent);
    }

    void SphereColliderViewModel::HandleSetRadius(const SetSphereColliderRadiusIntent& intent)
    {
        EntityID activeEntity = _selectionApi->GetSelectedEntity();
        if (activeEntity == NULL_ENTITY) return;

        _radiusDrag.Handle(intent.isDragging, intent.radius, _state.radius,
            [&](const float& v) { _colliderApi->SetSphereColliderRadius(activeEntity, v); },
            [&](const float& s, const float& e) { return std::make_shared<ChangeSphereColliderRadiusCommand>(_colliderApi, activeEntity, s, e); });
    }

    void SphereColliderViewModel::HandleSetLocalOffset(const SetSphereColliderLocalOffsetIntent& intent)
    {
        EntityID activeEntity = _selectionApi->GetSelectedEntity();
        if (activeEntity == NULL_ENTITY) return;

        _localOffsetDrag.Handle(intent.isDragging, intent.localOffset, _state.localOffset,
            [&](const glm::vec3& v) { _colliderApi->SetSphereColliderLocalOffset(activeEntity, v); },
            [&](const glm::vec3& s, const glm::vec3& e) { return std::make_shared<ChangeSphereColliderLocalOffsetCommand>(_colliderApi, activeEntity, s, e); });
    }
}