#pragma once
#include <string>
#include <vector>
#include <cstdint>
#include <glm/glm.hpp>
#include "EditorCore/Types/TextureHandle.h"

namespace Syn 
{
    struct ModelNode {
        uint32_t id;
        std::string name;
        std::string path;
        std::string icon;
        glm::vec2 uv0{ 0.0f, 0.0f };
        glm::vec2 uv1{ 1.0f, 1.0f };
        bool hasPreview = false;
    };

    struct ModelHierarchyNode {
        uint32_t modelId = 0xFFFFFFFF;
        int32_t descriptorIndex = -1;

        std::string name;
        std::string icon;

        uint32_t triangleCount = 0;
        bool isMeshNode = false;

        int depth = 0;
        bool isExpanded = true;
        bool hasChildren = false;
    };

    struct ModelHierarchyState {
        std::vector<ModelNode> filteredModels;
        std::vector<ModelHierarchyNode> flatNodes;

        uint32_t selectedModelId = 0xFFFFFFFF;
        int32_t selectedDescriptorIndex = -1;
        std::string searchQuery = "";

        TextureHandle atlasHandle = InvalidTextureHandle;
    };
}