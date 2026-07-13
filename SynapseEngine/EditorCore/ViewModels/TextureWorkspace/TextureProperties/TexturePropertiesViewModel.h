#pragma once
#include "EditorCore/ViewModels/IViewModel.h"
#include "TexturePropertiesState.h"
#include "TexturePropertiesIntent.h"
#include "EditorCore/Api/ITextureApi.h"

namespace Syn {
    class TexturePropertiesViewModel : public IViewModel<TexturePropertiesState, TexturePropertiesIntent> {
    public:
        TexturePropertiesViewModel(ITextureApi* textureApi);
        ~TexturePropertiesViewModel() override = default;

        const TexturePropertiesState& GetState() const override { return _state; }
        void SyncWithEngine() override;
        void Dispatch(const TexturePropertiesIntent& intent) override;
    private:
        ITextureApi* _textureApi = nullptr;
        TexturePropertiesState _state;
    };
}