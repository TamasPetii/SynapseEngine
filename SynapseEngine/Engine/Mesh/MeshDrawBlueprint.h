#pragma once
#include "Engine/SynApi.h"
#include "Engine/Render/PipelineRenderType.h"
#include <cstdint>
#include <vulkan/vulkan.h>

namespace Syn
{
    struct SYN_API MeshDrawBlueprint
    {
        VkDrawIndirectCommand traditionalCmd;
        VkDrawMeshTasksIndirectCommandEXT meshletCmd;
        PipelineRenderType pipelineRenderType;
    };
}