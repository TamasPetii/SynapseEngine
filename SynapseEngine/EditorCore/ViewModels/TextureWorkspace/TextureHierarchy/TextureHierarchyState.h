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