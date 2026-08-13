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
#include "ModelViewportState.h"
#include "ModelViewportIntent.h"
#include "EditorCore/Api/IRenderApi.h"
#include "EditorCore/Api/ISelectionApi.h"
#include "EditorCore/Api/ITransformApi.h"
#include "EditorCore/Api/ISettingsApi.h"
#include "EditorCore/Api/IModelApi.h"

namespace Syn {
    class ModelViewportViewModel : public IViewModel<ModelViewportState, ModelViewportIntent> {
    public:
        ModelViewportViewModel(IRenderApi* renderApi, ISelectionApi* selectionApi, ITransformApi* transformApi, ISettingsApi* settingsApi, IModelApi* modelApi);
        ~ModelViewportViewModel() override = default;

        const ModelViewportState& GetState() const override;

        void SyncWithEngine() override;
        void Dispatch(const ModelViewportIntent& intent) override;
    private:
        void HandlePickMesh(const PickMeshIntent& intent);
        void HandleResize(const ResizeModelViewportIntent& intent);
        void HandleChangeTarget(const ChangeModelTargetIntent& intent);
        void HandleGizmoTransform(const ApplyModelGizmoTransformIntent& intent);
        void HandleToggleDebugVisibility(const ToggleModelDebugVisibilityIntent& intent);
        void HandleChangeDebugVisibilityMode(const ChangeModelDebugVisibilityModeIntent& intent);
    private:
        IRenderApi* _renderApi = nullptr;
        ISelectionApi* _selectionApi = nullptr;
        ITransformApi* _transformApi = nullptr;
        ISettingsApi* _settingsApi = nullptr;
        IModelApi* _modelApi = nullptr;
        ModelViewportState _state;
    };
}