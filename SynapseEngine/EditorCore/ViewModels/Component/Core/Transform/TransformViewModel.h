#pragma once
#include "EditorCore/ViewModels/IViewModel.h"
#include "EditorCore/Interaction/DragInteraction.h"
#include "TransformState.h"
#include "TransformIntent.h"
#include "TransformCommands.h"
#include "EditorCore/API/ISelectionAPI.h"
#include "EditorCore/API/ITransformAPI.h"

namespace Syn {
    class TransformViewModel : public IViewModel<TransformState, TransformIntent> {
    public:
        TransformViewModel(ISelectionAPI* selectionApi, ITransformAPI* transformApi);
        ~TransformViewModel() override = default;

        const TransformState& GetState() const override;
        void SyncWithEngine() override;
        void Dispatch(const TransformIntent& intent) override;

    private:
        void HandleSetPosition(const SetPositionIntent& intent);
        void HandleSetRotation(const SetRotationIntent& intent);
        void HandleSetScale(const SetScaleIntent& intent);
    private:
        ISelectionAPI* _selectionApi = nullptr;
        ITransformAPI* _transformApi = nullptr;
        TransformState _state;

        DragInteraction<glm::vec3> _positionDrag;
        DragInteraction<glm::vec3> _rotationDrag;
        DragInteraction<glm::vec3> _scaleDrag;
    };
}