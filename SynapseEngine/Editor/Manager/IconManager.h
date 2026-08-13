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
#include "IIconManager.h"
#include "GuiTextureManager.h"
#include "Engine/Image/ImageManager.h"
#include <unordered_map>
#include <string>

namespace Syn {
    class IconManager : public IIconManager {
    public:
        IconManager(ImageManager* imageManager, GuiTextureManager* guiTextureManager);
        ~IconManager() override = default;

        void InitializeFontAwesome(ImGuiIO& io, const std::string& fontPath, float fontSize);
        void LoadEngineIcons(const std::string& iconDirectory);

        ImTextureID GetIconDescriptor(EditorIconType type) const override;
        ImFont* GetMainIconFont() const override { return _fontAwesome; }

    private:
        ImageManager* _imageManager = nullptr;
        GuiTextureManager* _guiTextureManager = nullptr;
        ImFont* _fontAwesome = nullptr;

        std::unordered_map<EditorIconType, ImTextureID> _iconCache;
    };
}