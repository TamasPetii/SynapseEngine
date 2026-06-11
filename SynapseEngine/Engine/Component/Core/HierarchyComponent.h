#pragma once
#include "Engine/SynApi.h"
#include "Engine/Registry/Entity.h"
#include "Engine/Component/Core/Component.h"

namespace Syn
{
    struct SYN_API HierarchyComponent : public Component
    {
        EntityID parent = NULL_ENTITY;
        EntityID firstChild = NULL_ENTITY;
        EntityID nextSibling = NULL_ENTITY;
        EntityID prevSibling = NULL_ENTITY;

        uint32_t depthLevel = 0;
        uint32_t topoIndex = NULL_INDEX;
    };
}