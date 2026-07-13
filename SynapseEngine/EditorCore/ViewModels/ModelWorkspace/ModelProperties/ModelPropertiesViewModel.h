#pragma once
#include "EditorCore/ViewModels/IViewModel.h"
#include "ModelPropertiesState.h"
#include "ModelPropertiesIntent.h"
#include "EditorCore/Api/IModelApi.h"

namespace Syn {
    class ModelPropertiesViewModel : public IViewModel<ModelPropertiesState, ModelPropertiesIntent> {
    public:
        ModelPropertiesViewModel(IModelApi* modelApi);
        ~ModelPropertiesViewModel() override = default;

        const ModelPropertiesState& GetState() const override { return _state; }
        void SyncWithEngine() override;
        void Dispatch(const ModelPropertiesIntent& intent) override;

    private:
        void UpdateState();

    private:
        IModelApi* _modelApi = nullptr;
        ModelPropertiesState _state;

        uint32_t _lastModelId = 0xFFFFFFFF;
        int32_t _lastDescriptorIndex = -1;
    };
}