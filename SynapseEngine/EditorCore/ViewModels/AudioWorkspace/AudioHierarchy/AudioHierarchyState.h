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
#include <glm/glm.hpp>
#include "EditorCore/Types/TextureHandle.h"

namespace Syn {
    struct AudioHierarchyNode {
        uint32_t id;
        std::string name;
        std::string path;
        std::string icon;

        bool hasPreview = false;
        glm::vec2 uv0{ 0.0f, 0.0f };
        glm::vec2 uv1{ 1.0f, 1.0f };
    };

    struct AudioHierarchyState {
        std::vector<AudioHierarchyNode> filteredNodes;
        uint32_t selectedAudioId = 0xFFFFFFFF;
        std::string searchQuery = "";

        TextureHandle atlasHandle = InvalidTextureHandle;
    };
}