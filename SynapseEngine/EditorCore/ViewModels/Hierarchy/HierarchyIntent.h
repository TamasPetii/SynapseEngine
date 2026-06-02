// EditorCore/ViewModels/Hierarchy/HierarchyIntent.h
#pragma once
#include <string>
#include <variant>
#include "EditorCore/Types/EntityHandle.h"

namespace Syn 
{
    struct HierarchySelectEntityIntent { 
        EntityID entity;
    };

    struct HierarchyToggleExpandIntent {
        EntityID entity;
        bool expand;
    };

    struct HierarchyToggleVisibilityIntent {
        EntityID entity;
        bool visible;
    };

    struct HierarchyReparentEntityIntent {
        EntityID child;
        EntityID newParent;
    };

    struct HierarchyCreateEntityIntent {
        std::string name; 
        EntityID parent;
    };

    struct HierarchyDestroyEntityIntent {
        EntityID entity;
    };

    struct HierarchyRefreshHierarchyIntent {
    };

    struct HierarchySetSearchQueryIntent {
        std::string query;
    };

    struct HierarchyExpandAllIntent {
    };

    struct HierarchyCollapseAllIntent {
    };

    using HierarchyIntent = std::variant<
        HierarchySelectEntityIntent,
        HierarchyToggleExpandIntent,
        HierarchyToggleVisibilityIntent,
        HierarchyReparentEntityIntent,
        HierarchyCreateEntityIntent,
        HierarchyDestroyEntityIntent,
        HierarchyRefreshHierarchyIntent,
        HierarchySetSearchQueryIntent,
        HierarchyExpandAllIntent,
        HierarchyCollapseAllIntent
    >;
}