#pragma once
#include "Engine/SynApi.h"
#include <cstdint>
#include <vector>

namespace Syn
{
    struct SYN_API MeshAllocationInfo
    {
        uint32_t descriptorIndex;
        uint32_t indirectIndex;
        uint32_t instanceOffset;
        uint32_t isMeshletPipeline;
    };

    struct SYN_API ModelAllocationInfo
    {
        uint32_t maxInstances;
        uint32_t meshAllocationOffset;
        uint32_t meshAllocationCount;
        uint32_t padding;
    };
}