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
#include "EditorCore/ViewModels/IViewModel.h"
#include "EditorCore/Interaction/DragInteraction.h"
#include "RigidBodyState.h"
#include "RigidBodyIntent.h"
#include "RigidBodyCommands.h"
#include "EditorCore/Api/ISelectionApi.h"
#include "EditorCore/Api/IRigidBodyApi.h"

namespace Syn {
    class RigidBodyViewModel : public IViewModel<RigidBodyState, RigidBodyIntent> {
    public:
        RigidBodyViewModel(ISelectionApi* selectionApi, IRigidBodyApi* rigidBodyApi);
        ~RigidBodyViewModel() override = default;

        const RigidBodyState& GetState() const override;
        void SyncWithEngine() override;
        void Dispatch(const RigidBodyIntent& intent) override;

    private:
        void HandleSetMotionType(const SetRigidBodyMotionTypeIntent& intent);
        void HandleSetMass(const SetRigidBodyMassIntent& intent);
        void HandleSetFriction(const SetRigidBodyFrictionIntent& intent);
        void HandleSetRestitution(const SetRigidBodyRestitutionIntent& intent);
        void HandleSetLayer(const SetRigidBodyLayerIntent& intent);

    private:
        ISelectionApi* _selectionApi = nullptr;
        IRigidBodyApi* _rigidBodyApi = nullptr;
        RigidBodyState _state;

        DragInteraction<float> _massDrag;
        DragInteraction<float> _frictionDrag;
        DragInteraction<float> _restitutionDrag;
    };
}