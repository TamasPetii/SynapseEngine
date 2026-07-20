#pragma once
#include "Engine/SynApi.h"
#include "Engine/Material/MaterialRenderType.h"
#include "Engine/Render/PipelineRenderType.h"
#include <cstdint>
#include <vector>

namespace Syn
{
    struct alignas(16) SYN_API MeshAllocationInfo
    {
        uint32_t descriptorIndex;
        uint32_t padding[3];

        uint32_t indirectIndices[PipelineRenderType::PipelineRenderTypeCount][MaterialRenderType::MaterialRenderTypeCount];
        uint32_t instanceOffsets[PipelineRenderType::PipelineRenderTypeCount][MaterialRenderType::MaterialRenderTypeCount];
        uint32_t activeTypes[PipelineRenderType::PipelineRenderTypeCount][MaterialRenderType::MaterialRenderTypeCount];
    };

    struct alignas(16) SYN_API ModelAllocationInfo
    {
        uint32_t maxInstances;
        uint32_t meshAllocationOffset;
        uint32_t meshAllocationCount;
        uint32_t padding;
    };
}