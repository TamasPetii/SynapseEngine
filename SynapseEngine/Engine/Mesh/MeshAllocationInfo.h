#pragma once
#include "Engine/SynApi.h"
#include "Engine/Material/MaterialRenderType.h"
#include <cstdint>
#include <vector>

namespace Syn
{
    struct alignas(16) SYN_API MeshAllocationInfo
    {
        uint32_t descriptorIndex;
        uint32_t isMeshletPipeline;
        uint32_t padding[2];

        uint32_t indirectIndices[MaterialRenderType::Count];
        uint32_t instanceOffsets[MaterialRenderType::Count];
        uint32_t activeTypes[MaterialRenderType::Count];
    };

    struct alignas(16) SYN_API ModelAllocationInfo
    {
        uint32_t maxInstances;
        uint32_t meshAllocationOffset;
        uint32_t meshAllocationCount;
        uint32_t padding;
    };
}