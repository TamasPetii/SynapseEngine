#pragma once
#include "EditorCore/ViewModels/IViewModel.h"
#include "EditorCore/Interaction/DragInteraction.h"
#include "TransformState.h"
#include "TransformIntent.h"
#include "TransformCommands.h"
#include "EditorCore/Api/ISelectionApi.h"
#include "EditorCore/Api/ITransformApi.h"

namespace Syn {
    class TransformViewModel : public IViewModel<TransformState, TransformIntent> {
    public:
        TransformViewModel(ISelectionApi* selectionApi, ITransformApi* transformApi);
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
        TransformState _state;

        DragInteraction<glm::vec3> _positionDrag;
        DragInteraction<glm::vec3> _rotationDrag;
        DragInteraction<glm::vec3> _scaleDrag;
    };
}