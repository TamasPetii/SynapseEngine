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
#include "BoxColliderState.h"
#include "BoxColliderIntent.h"
#include "BoxColliderCommands.h"
#include "EditorCore/Api/ISelectionApi.h"
#include "EditorCore/Api/IBoxColliderApi.h"

namespace Syn {
    class BoxColliderViewModel : public IViewModel<BoxColliderState, BoxColliderIntent> {
    public:
        BoxColliderViewModel(ISelectionApi* selectionApi, IBoxColliderApi* colliderApi);
        ~BoxColliderViewModel() override = default;

        const BoxColliderState& GetState() const override;
        void SyncWithEngine() override;
        void Dispatch(const BoxColliderIntent& intent) override;

    private:
        void HandleSetHalfExtents(const SetBoxColliderHalfExtentsIntent& intent);
        void HandleSetLocalOffset(const SetBoxColliderLocalOffsetIntent& intent);

    private:
        ISelectionApi* _selectionApi = nullptr;
        IBoxColliderApi* _colliderApi = nullptr;
        BoxColliderState _state;

        DragInteraction<glm::vec3> _halfExtentsDrag;
        DragInteraction<glm::vec3> _localOffsetDrag;
    };
}