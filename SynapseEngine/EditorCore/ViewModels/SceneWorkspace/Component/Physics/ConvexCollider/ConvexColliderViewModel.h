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