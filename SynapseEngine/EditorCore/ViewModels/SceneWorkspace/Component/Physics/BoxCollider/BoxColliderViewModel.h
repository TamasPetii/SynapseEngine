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