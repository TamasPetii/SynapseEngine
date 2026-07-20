#pragma once
#include "Engine/SynApi.h"
#include "Engine/Registry/Entity.h"
#include "Engine/Component/Core/Component.h"
#include <vector>

namespace Syn
{
    struct SYN_API PipelineOverrideComponent : public Component {
        EntityID sharedPipelineEntity = NULL_ENTITY;
        std::vector<uint32_t> pipelines;
    };
}