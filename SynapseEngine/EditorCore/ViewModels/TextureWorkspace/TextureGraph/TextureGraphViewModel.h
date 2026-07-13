#pragma once
#include "EditorCore/ViewModels/IViewModel.h"
#include "TextureGraphState.h"
#include "TextureGraphIntent.h"
#include "EditorCore/Api/ITextureApi.h"

namespace Syn {
    class TextureGraphViewModel : public IViewModel<TextureGraphState, TextureGraphIntent> {
    public:
        TextureGraphViewModel(ITextureApi* textureApi);
        ~TextureGraphViewModel() override = default;

        const TextureGraphState& GetState() const override { return _state; }

        void SyncWithEngine() override;
        void Dispatch(const TextureGraphIntent& intent) override;
    private:
        ITextureApi* _textureApi = nullptr;
        uint32_t _lastSelectedId = 0xFFFFFFFF;
        TextureGraphState _state;
    };
}