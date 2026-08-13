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
#include "AnimationViewportState.h"
#include "AnimationViewportIntent.h"
#include "EditorCore/Api/IRenderApi.h"
#include "EditorCore/Api/ISelectionApi.h"
#include "EditorCore/Api/ITransformApi.h"
#include "EditorCore/Api/ISettingsApi.h"
#include "EditorCore/Api/IAnimationApi.h"

namespace Syn {
    class AnimationViewportViewModel : public IViewModel<AnimationViewportState, AnimationViewportIntent> {
    public:
        AnimationViewportViewModel(IRenderApi* renderApi, ISelectionApi* selectionApi, ITransformApi* transformApi, ISettingsApi* settingsApi, IAnimationApi* animationApi);
        ~AnimationViewportViewModel() override = default;

        const AnimationViewportState& GetState() const override;

        void SyncWithEngine() override;
        void Dispatch(const AnimationViewportIntent& intent) override;
    private:
        void HandlePickMesh(const PickAnimationMeshIntent& intent);
        void HandleResize(const ResizeAnimationViewportIntent& intent);
        void HandleChangeTarget(const ChangeAnimationTargetIntent& intent);
        void HandleGizmoTransform(const ApplyAnimationGizmoTransformIntent& intent);
        void HandleToggleDebugVisibility(const ToggleAnimationDebugVisibilityIntent& intent);
        void HandleChangeDebugVisibilityMode(const ChangeAnimationDebugVisibilityModeIntent& intent);
    private:
        IRenderApi* _renderApi = nullptr;
        ISelectionApi* _selectionApi = nullptr;
        ITransformApi* _transformApi = nullptr;
        ISettingsApi* _settingsApi = nullptr;
        IAnimationApi* _animationApi = nullptr;
        AnimationViewportState _state;
    };
}