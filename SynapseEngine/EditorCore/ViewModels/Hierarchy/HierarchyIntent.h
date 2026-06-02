// EditorCore/ViewModels/Hierarchy/HierarchyIntent.h
#pragma once
#include <string>
#include <variant>
#include "EditorCore/Types/EntityHandle.h"

namespace Syn 
{
    struct SelectEntityIntent { 
        EntityID entity;
    };

    struct ToggleExpandIntent { 
        EntityID entity;
        bool expand;
    };

    struct ToggleVisibilityIntent { 
        EntityID entity;
        bool visible;
    };

    struct ReparentEntityIntent { 
        EntityID child;
        EntityID newParent;
    };

    struct CreateEntityIntent { 
        std::string name; 
        EntityID parent;
    };

    struct DestroyEntityIntent { 
        EntityID entity;
    };

    struct RefreshHierarchyIntent {
    };

    struct SetSearchQueryIntent {
        std::string query;
    };

    struct ExpandAllIntent {
    };

    struct CollapseAllIntent {
    };

    using HierarchyIntent = std::variant<
        SelectEntityIntent,
        ToggleExpandIntent,
        ToggleVisibilityIntent,
        ReparentEntityIntent,
        CreateEntityIntent,
        DestroyEntityIntent,
        RefreshHierarchyIntent,
        SetSearchQueryIntent,
        ExpandAllIntent,
        CollapseAllIntent
    >;
}