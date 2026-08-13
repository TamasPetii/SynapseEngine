// Copyright (C) 2026 Tamás Péter
// This file is part of SynapseEngine.
//
// SynapseEngine is free software: you can redistribute it and/or modify
// it under the terms of the GNU Affero General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// SynapseEngine is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License
// along with SynapseEngine. If not, see <https://www.gnu.org/licenses/>.

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