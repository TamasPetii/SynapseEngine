// Copyright (C) 2026 Tamás Péter
// This file is part of SynapseEngine.
//
// SynapseEngine is free software: you can redistribute it and/or modify
// it under the terms of the GNU Affero General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// SynapseEngine is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License
// along with SynapseEngine. If not, see <https://www.gnu.org/licenses/>.

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