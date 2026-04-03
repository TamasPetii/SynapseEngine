#pragma once
#include "Engine/SynApi.h"
#include <cstdint>
#include <vulkan/vulkan.h>

namespace Syn
{
    struct SYN_API MeshDrawBlueprint
    {
        static constexpr uint32_t PIPELINE_TRADITIONAL = 0;
        static constexpr uint32_t PIPELINE_MESHLET = 1;

        VkDrawIndirectCommand traditionalCmd;
        VkDrawMeshTasksIndirectCommandEXT meshletCmd;
        uint32_t isMeshletPipeline;
    };
}