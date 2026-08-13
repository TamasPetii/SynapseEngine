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
#include "SphereColliderState.h"
#include "SphereColliderIntent.h"
#include "SphereColliderCommands.h"
#include "EditorCore/Api/ISelectionApi.h"
#include "EditorCore/Api/ISphereColliderApi.h"

namespace Syn {
    class SphereColliderViewModel : public IViewModel<SphereColliderState, SphereColliderIntent> {
    public:
        SphereColliderViewModel(ISelectionApi* selectionApi, ISphereColliderApi* colliderApi);
        ~SphereColliderViewModel() override = default;

        const SphereColliderState& GetState() const override;
        void SyncWithEngine() override;
        void Dispatch(const SphereColliderIntent& intent) override;

    private:
        void HandleSetRadius(const SetSphereColliderRadiusIntent& intent);
        void HandleSetLocalOffset(const SetSphereColliderLocalOffsetIntent& intent);

    private:
        ISelectionApi* _selectionApi = nullptr;
        ISphereColliderApi* _colliderApi = nullptr;
        SphereColliderState _state;

        DragInteraction<float> _radiusDrag;
        DragInteraction<glm::vec3> _localOffsetDrag;
    };
}