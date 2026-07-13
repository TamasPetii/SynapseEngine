#pragma once
#include "EditorCore/ViewModels/IViewModel.h"
#include "ModelComponentState.h"
#include "ModelComponentIntent.h"
#include "ModelComponentCommands.h"
#include "EditorCore/Api/ISelectionApi.h"
#include "EditorCore/Api/IModelComponentApi.h"

namespace Syn {
    class ModelComponentViewModel : public IViewModel<ModelComponentState, ModelComponentIntent> {
    public:
        ModelComponentViewModel(ISelectionApi* selectionApi, IModelComponentApi* modelApi);
        ~ModelComponentViewModel() override = default;

        const ModelComponentState& GetState() const override;
        void SyncWithEngine() override;
        void Dispatch(const ModelComponentIntent& intent) override;

    private:
        ISelectionApi* _selectionApi = nullptr;
        IModelComponentApi* _modelApi = nullptr;
        ModelComponentState _state;
    };
}