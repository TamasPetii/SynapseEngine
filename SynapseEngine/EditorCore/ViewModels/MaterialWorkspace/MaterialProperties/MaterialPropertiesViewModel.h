#pragma once
#include "EditorCore/ViewModels/IViewModel.h"
#include "MaterialPropertiesState.h"
#include "MaterialPropertiesIntent.h"
#include "EditorCore/Api/IMaterialApi.h"
#include "EditorCore/Api/ITextureApi.h"

namespace Syn {
    class MaterialPropertiesViewModel : public IViewModel<MaterialPropertiesState, MaterialPropertiesIntent> {
    public:
        MaterialPropertiesViewModel(IMaterialApi* materialApi, ITextureApi* textureApi);
        ~MaterialPropertiesViewModel() override = default;

        const MaterialPropertiesState& GetState() const override { return _state; }
        void SyncWithEngine() override;
        void Dispatch(const MaterialPropertiesIntent& intent) override;
    private:
        IMaterialApi* _materialApi = nullptr;
        ITextureApi* _textureApi = nullptr;
        MaterialPropertiesState _state;
    };
}