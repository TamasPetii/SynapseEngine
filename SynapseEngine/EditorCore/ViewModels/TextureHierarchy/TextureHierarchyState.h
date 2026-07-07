#pragma once
#include <string>
#include <vector>
#include <cstdint>

namespace Syn {
    struct TextureNode {
        uint32_t id;
        std::string name;
        std::string icon;
    };

    struct TextureHierarchyState {
        std::vector<TextureNode> filteredNodes;
        uint32_t selectedTexture = 0xFFFFFFFF;
        std::string searchQuery = "";
    };
}