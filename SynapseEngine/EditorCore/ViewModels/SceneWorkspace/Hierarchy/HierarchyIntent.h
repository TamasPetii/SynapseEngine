// EditorCore/ViewModels/Hierarchy/HierarchyIntent.h
#pragma once
#include <string>
#include <variant>
#include "EditorCore/Types/EntityHandle.h"
#include "EditorCore/Api/IHierarchyApi.h"

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
        EntityTemplate type;
        EntityID parent;
    };

    struct HierarchyCopyEntityIntent {
        EntityID entity;
    };

    struct HierarchyFullCopyEntityIntent {
        EntityID entity;
    };

    struct HierarchyDestroyEntityIntent {
        EntityID entity;
    };

    struct HierarchyDestroyKeepChildrenIntent {
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
        HierarchyCopyEntityIntent,
        HierarchyFullCopyEntityIntent,
        HierarchyDestroyEntityIntent,
        HierarchyDestroyKeepChildrenIntent,
        HierarchyRefreshHierarchyIntent,
        HierarchySetSearchQueryIntent,
        HierarchyExpandAllIntent,
        HierarchyCollapseAllIntent
    >;
}