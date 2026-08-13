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
#include "SpotLightState.h"
#include "SpotLightIntent.h"
#include "SpotLightCommands.h"
#include "EditorCore/Api/ISelectionApi.h"
#include "EditorCore/Api/ISpotLightApi.h"

namespace Syn {
    class SpotLightViewModel : public IViewModel<SpotLightState, SpotLightIntent> {
    public:
        SpotLightViewModel(ISelectionApi* selectionApi, ISpotLightApi* lightApi);
        ~SpotLightViewModel() override = default;

        const SpotLightState& GetState() const override;
        void SyncWithEngine() override;
        void Dispatch(const SpotLightIntent& intent) override;

    private:
        ISelectionApi* _selectionApi = nullptr;
        ISpotLightApi* _lightApi = nullptr;
        SpotLightState _state;

        DragInteraction<glm::vec3> _colorDrag;
        DragInteraction<float> _strengthDrag;
        DragInteraction<float> _rangeDrag;
        DragInteraction<float> _weakenDrag;
        DragInteraction<float> _innerAngleDrag;
        DragInteraction<float> _outerAngleDrag;
        DragInteraction<float> _nearPlaneDrag;
        DragInteraction<float> _farPlaneDrag;
    };
}