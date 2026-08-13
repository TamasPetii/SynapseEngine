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
#include "TransformState.h"
#include "TransformIntent.h"
#include "TransformCommands.h"
#include "EditorCore/Api/ISelectionApi.h"
#include "EditorCore/Api/ITransformApi.h"
#include "EditorCore/Api/IHierarchyApi.h"

namespace Syn {
    class TransformViewModel : public IViewModel<TransformState, TransformIntent> {
    public:
        TransformViewModel(ISelectionApi* selectionApi, ITransformApi* transformApi, IHierarchyApi* hierarchyApi);
        ~TransformViewModel() override = default;

        const TransformState& GetState() const override;
        void SyncWithEngine() override;
        void Dispatch(const TransformIntent& intent) override;

    private:
        void HandleSetPosition(const SetPositionIntent& intent);
        void HandleSetRotation(const SetRotationIntent& intent);
        void HandleSetScale(const SetScaleIntent& intent);
    private:
        ISelectionApi* _selectionApi = nullptr;
        ITransformApi* _transformApi = nullptr;
        IHierarchyApi* _hierarchyApi = nullptr;
        TransformState _state;

        DragInteraction<glm::vec3> _positionDrag;
        DragInteraction<glm::vec3> _rotationDrag;
        DragInteraction<glm::vec3> _scaleDrag;
    };
}