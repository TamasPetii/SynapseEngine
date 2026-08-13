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
#include "AnimationState.h"
#include "AnimationIntent.h"
#include "AnimationCommands.h"
#include "EditorCore/Api/ISelectionApi.h"
#include "EditorCore/Api/IAnimationCompApi.h"

namespace Syn {
    class AnimationViewModel : public IViewModel<AnimationState, AnimationIntent> {
    public:
        AnimationViewModel(ISelectionApi* selectionApi, IAnimationCompApi* animApi);
        ~AnimationViewModel() override = default;

        const AnimationState& GetState() const override;
        void SyncWithEngine() override;
        void Dispatch(const AnimationIntent& intent) override;

    private:
        ISelectionApi* _selectionApi = nullptr;
        IAnimationCompApi* _animApi = nullptr;
        AnimationState _state;

        DragInteraction<float> _speedDrag;
    };
}