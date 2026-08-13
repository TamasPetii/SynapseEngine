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
#include "ViewportState.h"
#include "ViewportIntent.h"
#include "EditorCore/Api/IRenderApi.h"
#include "EditorCore/Api/ISelectionApi.h"
#include "EditorCore/Api/ITransformApi.h"
#include "EditorCore/Api/ISettingsApi.h"
#include "EditorCore/Api/IHierarchyApi.h"

namespace Syn {
    class ViewportViewModel : public IViewModel<ViewportState, ViewportIntent> {
    public:
        ViewportViewModel(IRenderApi* renderApi, ISelectionApi* selectionApi, ITransformApi* transformApi, ISettingsApi* settingsApi, IHierarchyApi* hierarchyApi);
        ~ViewportViewModel() override = default;

        const ViewportState& GetState() const override;

        void SyncWithEngine() override;
        void Dispatch(const ViewportIntent& intent) override;

    private:
        void HandlePickEntity(const PickEntityIntent& intent);
        void HandleResize(const ResizeViewportIntent& intent);
        void HandleChangeTarget(const ChangeTargetIntent& intent);
        void HandleGizmoTransform(const ApplyGizmoTransformIntent& intent);
        void HandleToggleDebugVisibility(const ToggleDebugVisibilityIntent& intent);
        void HandleChangeDebugVisibilityMode(const ChangeDebugVisibilityModeIntent& intent);

    private:
        IRenderApi* _renderApi = nullptr;
        ISelectionApi* _selectionApi = nullptr;
        ITransformApi* _transformApi = nullptr;
        ISettingsApi* _settingsApi = nullptr;
        IHierarchyApi* _hierarchyApi = nullptr;
        ViewportState _state;
    };
}