#include "TexturePropertiesViewModel.h"

namespace Syn {
    TexturePropertiesViewModel::TexturePropertiesViewModel(ITextureApi* textureApi)
        : _textureApi(textureApi)
    {}

    void TexturePropertiesViewModel::SyncWithEngine() {
        if (!_textureApi) return;

        uint32_t selectedId = _textureApi->GetSelectedTexture();

        if (selectedId == INVALID_TEXTURE_ID) {
            _state.hasSelection = false;
            return;
        }

        CpuTextureData texData;
        if (_textureApi->GetTextureData(selectedId, texData)) {
            _state.hasSelection = true;
            _state.width = texData.width;
            _state.height = texData.height;
            _state.depth = texData.depth;
            _state.mipLevels = texData.mipLevels;
            _state.format = static_cast<uint32_t>(texData.format);
            _state.isCompressed = texData.isCompressed;
        }
        else {
            _state.hasSelection = false;
        }
    }

    void TexturePropertiesViewModel::Dispatch(const TexturePropertiesIntent& intent) {
        
    }
}