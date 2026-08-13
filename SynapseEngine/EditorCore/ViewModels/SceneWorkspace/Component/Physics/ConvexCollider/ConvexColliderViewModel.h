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
#include "ConvexColliderState.h"
#include "ConvexColliderIntent.h"
#include "ConvexColliderCommands.h"
#include "EditorCore/Api/ISelectionApi.h"
#include "EditorCore/Api/IConvexColliderApi.h"

namespace Syn {
    class ConvexColliderViewModel : public IViewModel<ConvexColliderState, ConvexColliderIntent> {
    public:
        ConvexColliderViewModel(ISelectionApi* selectionApi, IConvexColliderApi* colliderApi);
        ~ConvexColliderViewModel() override = default;

        const ConvexColliderState& GetState() const override;
        void SyncWithEngine() override;
        void Dispatch(const ConvexColliderIntent& intent) override;

    private:
        void HandleSetTargetLodLevel(const SetConvexColliderTargetLodLevelIntent& intent);
        void HandleSetLocalOffset(const SetConvexColliderLocalOffsetIntent& intent);

    private:
        ISelectionApi* _selectionApi = nullptr;
        IConvexColliderApi* _colliderApi = nullptr;
        ConvexColliderState _state;

        DragInteraction<glm::vec3> _localOffsetDrag;
    };
}