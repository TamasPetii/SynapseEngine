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
#include <string>
#include <vector>
#include <cstdint>

#include "EditorCore/Types/TextureHandle.h"

namespace Syn {
    struct TextureNode {
        uint32_t id;
        std::string name;
        std::string path;
        std::string icon;
        TextureHandle handle = InvalidTextureHandle;
    };

    struct TextureHierarchyState {
        std::vector<TextureNode> filteredNodes;
        uint32_t selectedTexture = 0xFFFFFFFF;
        std::string searchQuery = "";
    };
}