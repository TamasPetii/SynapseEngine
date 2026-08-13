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
#include "CapsuleColliderState.h"
#include "CapsuleColliderIntent.h"
#include "CapsuleColliderCommands.h"
#include "EditorCore/Api/ISelectionApi.h"
#include "EditorCore/Api/ICapsuleColliderApi.h"

namespace Syn {
    class CapsuleColliderViewModel : public IViewModel<CapsuleColliderState, CapsuleColliderIntent> {
    public:
        CapsuleColliderViewModel(ISelectionApi* selectionApi, ICapsuleColliderApi* colliderApi);
        ~CapsuleColliderViewModel() override = default;

        const CapsuleColliderState& GetState() const override;
        void SyncWithEngine() override;
        void Dispatch(const CapsuleColliderIntent& intent) override;

    private:
        void HandleSetRadius(const SetCapsuleColliderRadiusIntent& intent);
        void HandleSetHalfHeight(const SetCapsuleColliderHalfHeightIntent& intent);
        void HandleSetLocalOffset(const SetCapsuleColliderLocalOffsetIntent& intent);

    private:
        ISelectionApi* _selectionApi = nullptr;
        ICapsuleColliderApi* _colliderApi = nullptr;
        CapsuleColliderState _state;

        DragInteraction<float> _radiusDrag;
        DragInteraction<float> _halfHeightDrag;
        DragInteraction<glm::vec3> _localOffsetDrag;
    };
}