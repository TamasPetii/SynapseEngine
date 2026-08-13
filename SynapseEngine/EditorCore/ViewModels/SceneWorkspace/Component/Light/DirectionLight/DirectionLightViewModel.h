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
#include "DirectionLightState.h"
#include "DirectionLightIntent.h"
#include "DirectionLightCommands.h"
#include "EditorCore/Api/ISelectionApi.h"
#include "EditorCore/Api/IDirectionLightApi.h"

namespace Syn {
    class DirectionLightViewModel : public IViewModel<DirectionLightState, DirectionLightIntent> {
    public:
        DirectionLightViewModel(ISelectionApi* selectionApi, IDirectionLightApi* lightApi);
        ~DirectionLightViewModel() override = default;

        const DirectionLightState& GetState() const override;
        void SyncWithEngine() override;
        void Dispatch(const DirectionLightIntent& intent) override;

    private:
        void HandleSetColor(const SetLightColorIntent& intent);
        void HandleSetStrength(const SetLightStrengthIntent& intent);
        void HandleSetUseShadow(const SetLightUseShadowIntent& intent);
        void HandleSetFarPlane(const SetShadowFarPlaneIntent& intent);
        void HandleSetCascadeSplits(const SetCascadeSplitsIntent& intent);

    private:
        ISelectionApi* _selectionApi = nullptr;
        IDirectionLightApi* _lightApi = nullptr;
        DirectionLightState _state;

        DragInteraction<glm::vec3> _colorDrag;
        DragInteraction<float> _strengthDrag;
        DragInteraction<float> _farPlaneDrag;
        DragInteraction<glm::vec4> _cascadeSplitsDrag;
    };
}