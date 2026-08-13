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

#include "RigidBodyViewModel.h"

namespace Syn
{
    RigidBodyViewModel::RigidBodyViewModel(ISelectionApi* selectionApi, IRigidBodyApi* rigidBodyApi)
        : _selectionApi(selectionApi), _rigidBodyApi(rigidBodyApi)
    {}

    const RigidBodyState& RigidBodyViewModel::GetState() const
    {
        return _state;
    }

    void RigidBodyViewModel::SyncWithEngine()
    {
        if (!_selectionApi || !_rigidBodyApi)
            return;

        EntityID activeEntity = _selectionApi->GetSelectedEntity();

        if (activeEntity != NULL_ENTITY && _rigidBodyApi->HasRigidBody(activeEntity))
        {
            _state.hasComponent = true;

            _state.motionType = _rigidBodyApi->GetRigidBodyMotionType(activeEntity);
            _state.layer = _rigidBodyApi->GetRigidBodyLayer(activeEntity);

            if (!_massDrag.IsDragging())
                _state.mass = _rigidBodyApi->GetRigidBodyMass(activeEntity);
            if (!_frictionDrag.IsDragging())
                _state.friction = _rigidBodyApi->GetRigidBodyFriction(activeEntity);
            if (!_restitutionDrag.IsDragging())
                _state.restitution = _rigidBodyApi->GetRigidBodyRestitution(activeEntity);
        }
        else
        {
            _state.hasComponent = false;
        }
    }

    void RigidBodyViewModel::Dispatch(const RigidBodyIntent& intent)
    {
        std::visit([this](auto&& arg)
            {
                using T = std::decay_t<decltype(arg)>;

                if constexpr (std::is_same_v<T, SetRigidBodyMotionTypeIntent>)      HandleSetMotionType(arg);
                else if constexpr (std::is_same_v<T, SetRigidBodyMassIntent>)       HandleSetMass(arg);
                else if constexpr (std::is_same_v<T, SetRigidBodyFrictionIntent>)   HandleSetFriction(arg);
                else if constexpr (std::is_same_v<T, SetRigidBodyRestitutionIntent>) HandleSetRestitution(arg);
                else if constexpr (std::is_same_v<T, SetRigidBodyLayerIntent>)      HandleSetLayer(arg); }, intent);
    }

    void RigidBodyViewModel::HandleSetMotionType(const SetRigidBodyMotionTypeIntent& intent)
    {
        EntityID activeEntity = _selectionApi->GetSelectedEntity();
        if (activeEntity == NULL_ENTITY) return;

        _state.motionType = intent.motionType;
        _rigidBodyApi->SetRigidBodyMotionType(activeEntity, intent.motionType);
    }

    void RigidBodyViewModel::HandleSetLayer(const SetRigidBodyLayerIntent& intent)
    {
        EntityID activeEntity = _selectionApi->GetSelectedEntity();
        if (activeEntity == NULL_ENTITY) return;

        _state.layer = intent.layer;
        _rigidBodyApi->SetRigidBodyLayer(activeEntity, intent.layer);
    }

    void RigidBodyViewModel::HandleSetMass(const SetRigidBodyMassIntent& intent)
    {
        EntityID activeEntity = _selectionApi->GetSelectedEntity();
        if (activeEntity == NULL_ENTITY) return;

        _massDrag.Handle(intent.isDragging, intent.mass, _state.mass,
            [&](const float& v) { _rigidBodyApi->SetRigidBodyMass(activeEntity, v); },
            [&](const float& s, const float& e) { return std::make_shared<ChangeRigidBodyMassCommand>(_rigidBodyApi, activeEntity, s, e); });
    }

    void RigidBodyViewModel::HandleSetFriction(const SetRigidBodyFrictionIntent& intent)
    {
        EntityID activeEntity = _selectionApi->GetSelectedEntity();
        if (activeEntity == NULL_ENTITY) return;

        _frictionDrag.Handle(intent.isDragging, intent.friction, _state.friction,
            [&](const float& v) { _rigidBodyApi->SetRigidBodyFriction(activeEntity, v); },
            [&](const float& s, const float& e) { return std::make_shared<ChangeRigidBodyFrictionCommand>(_rigidBodyApi, activeEntity, s, e); });
    }

    void RigidBodyViewModel::HandleSetRestitution(const SetRigidBodyRestitutionIntent& intent)
    {
        EntityID activeEntity = _selectionApi->GetSelectedEntity();
        if (activeEntity == NULL_ENTITY) return;

        _restitutionDrag.Handle(intent.isDragging, intent.restitution, _state.restitution,
            [&](const float& v) { _rigidBodyApi->SetRigidBodyRestitution(activeEntity, v); },
            [&](const float& s, const float& e) { return std::make_shared<ChangeRigidBodyRestitutionCommand>(_rigidBodyApi, activeEntity, s, e); });
    }
}