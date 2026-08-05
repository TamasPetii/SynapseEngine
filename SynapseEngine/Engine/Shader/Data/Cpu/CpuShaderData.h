#pragma once
#include "Engine/SynApi.h"
#include "Engine/Vk/VkCommon.h"
#include "Engine/Vk/Shader/ShaderReflectionData.h"
#include <string>

namespace Syn 
{
    struct SYN_API CpuShaderData 
    {
        VkShaderStageFlagBits stage = VK_SHADER_STAGE_FLAG_BITS_MAX_ENUM;
        std::string identifier;
        ShaderResources reflection;
    };
}