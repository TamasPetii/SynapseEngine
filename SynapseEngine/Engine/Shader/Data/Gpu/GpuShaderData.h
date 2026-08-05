#pragma once
#include "Engine/SynApi.h"
#include "Engine/Vk/VkCommon.h"
#include "Engine/Vk/Shader/ShaderReflectionData.h"
#include <vector>
#include <cstdint>

namespace Syn 
{
    struct SYN_API GpuShaderData 
    {
        std::vector<uint32_t> spirv;
		ShaderResources reflect;
        VkShaderStageFlagBits stage = VK_SHADER_STAGE_FLAG_BITS_MAX_ENUM;
    };
}