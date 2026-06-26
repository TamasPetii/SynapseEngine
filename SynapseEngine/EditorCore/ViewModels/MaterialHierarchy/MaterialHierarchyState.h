#pragma once
#include <string>
#include <vector>

namespace Syn {
    struct MaterialNode {
        uint32_t id;
        std::string name;
        std::string icon;
    };

    struct MaterialHierarchyState {
        std::vector<MaterialNode> filteredNodes;
        uint32_t selectedMaterial = 0xFFFFFFFF;
        std::string searchQuery = "";
    };
}