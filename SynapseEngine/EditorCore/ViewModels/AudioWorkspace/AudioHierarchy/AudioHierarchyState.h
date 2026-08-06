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