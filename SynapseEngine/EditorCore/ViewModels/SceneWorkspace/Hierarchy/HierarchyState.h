#pragma once
#include <string>
#include <vector>
#include "EditorCore/Types/EntityHandle.h"

namespace Syn {
    struct HierarchyNode {
        EntityID id;
        std::string name;
        std::string icon;
        int depth;
        bool hasChildren;
        bool isExpanded;
        bool isVisible;
    };

    struct HierarchyState {
        std::vector<HierarchyNode> flatNodes;
        EntityID selectedEntity = NULL_ENTITY;
        std::string searchQuery = "";
    };
}