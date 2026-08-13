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

#include "IconManager.h"
#include "EditorIcons.h"
#include "Engine/Image/SamplerNames.h"
#include "Engine/Vk/Image/ImageViewNames.h"
#include "Engine/Utils/PathUtils.h"

namespace Syn {
    IconManager::IconManager(ImageManager* imageManager, GuiTextureManager* guiTextureManager)
        : _imageManager(imageManager), _guiTextureManager(guiTextureManager) {}

    void IconManager::InitializeFontAwesome(ImGuiIO& io, const std::string& fontPath, float fontSize) {
        static const ImWchar icons_ranges[] = { ICON_MIN_FA, ICON_MAX_16_FA, 0 };
        ImFontConfig icons_config;
        icons_config.MergeMode = true;
        icons_config.PixelSnapH = true;
        icons_config.GlyphOffset.y = 2.5f;

        _fontAwesome = io.Fonts->AddFontFromFileTTF(fontPath.c_str(), fontSize, &icons_config, icons_ranges);
    }

    void IconManager::LoadEngineIcons(const std::string& iconDirectory) {
        if (!_imageManager || !_guiTextureManager) return;

        Vk::Sampler* sampler = _imageManager->GetSampler(SamplerNames::LinearClampEdge);
        if (!sampler) return;

        auto loadAndRegister = [&](EditorIconType type, const std::string& fileName) {
            std::string fullPath = PathUtils::GetAbsolutePathString(iconDirectory + "/" + fileName);

            uint32_t imageId = _imageManager->LoadImageSync(fullPath);
            auto texture = _imageManager->GetResource(imageId);

            if (texture && texture->image) {
                TextureHandle handle = _guiTextureManager->RegisterTexture(
                    texture->image->GetView(Vk::ImageViewNames::Default),
                    sampler->Handle()
                );
                _iconCache[type] = _guiTextureManager->GetImGuiTextureID(handle);
            }
            };

        loadAndRegister(EditorIconType::Folder, "folder.png");
        loadAndRegister(EditorIconType::File, "txt.png");
        loadAndRegister(EditorIconType::Image, "png.png");
        loadAndRegister(EditorIconType::Code, "code.png");
        loadAndRegister(EditorIconType::Model, "obj.png");
        loadAndRegister(EditorIconType::Sound, "mp3.png");
    }

    ImTextureID IconManager::GetIconDescriptor(EditorIconType type) const {
        auto it = _iconCache.find(type);
        if (it != _iconCache.end()) {
            return it->second;
        }
        return 0;
    }
}