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

#pragma once
#include "EditorCore/Api/ITextureApi.h"
#include "Engine/Image/ImageManager.h"
#include "Editor/Manager/GuiTextureManager.h"

namespace Syn {
    class TextureApiImpl : public ITextureApi {
    public:
        TextureApiImpl(ImageManager* imageManager, GuiTextureManager* guiTextureManager) 
            : _imageManager(imageManager), _guiTextureManager(guiTextureManager) {}

        std::vector<TextureItemData> GetAllTextures() override;
        std::vector<SamplerItemData> GetAllSamplers() const override;

        uint32_t GetSelectedTexture() const override;
        void SetSelectedTexture(uint32_t id) override;

        bool GetTextureData(uint32_t id, CpuTextureData& outData) const override;
        uint64_t GetVersion() const override;
        TextureHandle GetTextureHandle(uint32_t id) override;
    private:
        ImageManager* _imageManager;
        GuiTextureManager* _guiTextureManager;
        uint32_t _selectedTexture = INVALID_TEXTURE_ID;
        std::unordered_map<uint32_t, TextureHandle> _textureHandleCache;
    };
}