#pragma once
#include "Engine/SynApi.h"
#include "Engine/Registry/Entity.h"
#include "Engine/Mesh/Data/StaticMesh.h"
#include "BaseComponent/RenderStateComponent.h"

namespace Syn
{
    struct SYN_API MaterialOverrideComponent {
        std::vector<uint32_t> materials;
        EntityID sharedMaterialEntity = NULL_ENTITY;
    };
}



