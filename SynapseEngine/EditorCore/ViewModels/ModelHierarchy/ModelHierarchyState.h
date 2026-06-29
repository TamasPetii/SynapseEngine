#pragma once
#include <string>
#include <vector>
#include <cstdint>

namespace Syn {
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
        std::vector<ModelHierarchyNode> flatNodes;
        uint32_t selectedModelId = 0xFFFFFFFF;
        int32_t selectedDescriptorIndex = -1;
        std::string searchQuery = "";
    };
}