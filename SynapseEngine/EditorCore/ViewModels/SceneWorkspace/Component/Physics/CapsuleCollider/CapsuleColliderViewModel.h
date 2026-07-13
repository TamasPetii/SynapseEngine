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