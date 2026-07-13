#pragma once
#include <string>
#include <vector>
#include <cstdint>
#include <glm/glm.hpp>
#include "EditorCore/Types/TextureHandle.h"

namespace Syn {
    struct MaterialNode {
        uint32_t id;
        std::string name;
        std::string path;
        std::string icon;
        glm::vec2 uv0{ 0.0f, 0.0f };
        glm::vec2 uv1{ 1.0f, 1.0f };
        bool hasPreview = false;
    };

    struct MaterialHierarchyState {
        std::vector<MaterialNode> filteredNodes;
        uint32_t selectedMaterial = 0xFFFFFFFF;
        std::string searchQuery = "";
        TextureHandle atlasHandle = InvalidTextureHandle;
    };
}