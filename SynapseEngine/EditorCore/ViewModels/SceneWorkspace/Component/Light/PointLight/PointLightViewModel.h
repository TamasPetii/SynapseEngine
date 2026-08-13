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
#include "PointLightState.h"
#include "PointLightIntent.h"
#include "PointLightCommands.h"
#include "EditorCore/Api/ISelectionApi.h"
#include "EditorCore/Api/IPointLightApi.h"

namespace Syn {
    class PointLightViewModel : public IViewModel<PointLightState, PointLightIntent> {
    public:
        PointLightViewModel(ISelectionApi* selectionApi, IPointLightApi* lightApi);
        ~PointLightViewModel() override = default;

        const PointLightState& GetState() const override;
        void SyncWithEngine() override;
        void Dispatch(const PointLightIntent& intent) override;

    private:
        void HandleSetColor(const SetPointLightColorIntent& intent);
        void HandleSetStrength(const SetPointLightStrengthIntent& intent);
        void HandleSetUseShadow(const SetPointLightUseShadowIntent& intent);
        void HandleSetRadius(const SetPointLightRadiusIntent& intent);
        void HandleSetWeaken(const SetPointLightWeakenIntent& intent);
        void HandleSetNearPlane(const SetPointLightShadowNearIntent& intent);
        void HandleSetFarPlane(const SetPointLightShadowFarIntent& intent);

    private:
        ISelectionApi* _selectionApi = nullptr;
        IPointLightApi* _lightApi = nullptr;
        PointLightState _state;

        DragInteraction<glm::vec3> _colorDrag;
        DragInteraction<float> _strengthDrag;
        DragInteraction<float> _radiusDrag;
        DragInteraction<float> _weakenDrag;
        DragInteraction<float> _nearPlaneDrag;
        DragInteraction<float> _farPlaneDrag;
    };
}