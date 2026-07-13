#include "TextureGraphViewModel.h"

namespace Syn {
    TextureGraphViewModel::TextureGraphViewModel(ITextureApi* textureApi)
        : _textureApi(textureApi)
    {}

    void TextureGraphViewModel::SyncWithEngine() {
        if (!_textureApi) return;

        uint32_t selectedId = _textureApi->GetSelectedTexture();

        if (selectedId != _lastSelectedId) {
            _lastSelectedId = selectedId;

            if (selectedId == 0xFFFFFFFF) {
                _state.previewNode.isVisible = false;
                _state.previewNode.textureHandle = InvalidTextureHandle;
            }
            else {
                CpuTextureData texData;
                if (_textureApi->GetTextureData(selectedId, texData)) {
                    _state.previewNode.isVisible = true;
                    _state.previewNode.engineTextureId = selectedId;
                    _state.previewNode.width = texData.width;
                    _state.previewNode.height = texData.height;
                    _state.previewNode.textureHandle = _textureApi->GetTextureHandle(selectedId);
                }
            }
        }
    }

    void TextureGraphViewModel::Dispatch(const TextureGraphIntent& intent) {

    }
}