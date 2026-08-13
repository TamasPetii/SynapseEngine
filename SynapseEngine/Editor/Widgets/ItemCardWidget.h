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
#include <imgui.h>
#include <functional>

namespace Syn::UI {
    struct ItemCardEvents {
        std::function<void()> onClick;
        std::function<void()> onDoubleClick;
        std::function<void()> onDragDropSource;
    };

    struct ItemCardDesc {
        const char* id = nullptr;
        const char* title = "";

        ImTextureID texture = 0;
        ImVec2 uv0 = ImVec2(0.0f, 0.0f);
        ImVec2 uv1 = ImVec2(1.0f, 1.0f);
        ImVec4 imageTint = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);

        bool selected = false;
        ItemCardEvents events;
    };

    bool ItemCard(const ItemCardDesc& desc, float thumbnailSize);
    float ItemCardWidth(float thumbnailSize);
    float ItemCardHeight(float thumbnailSize);
}