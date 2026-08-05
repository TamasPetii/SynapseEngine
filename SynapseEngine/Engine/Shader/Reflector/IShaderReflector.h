#pragma once
#include "Engine/SynApi.h"
#include "Engine/Vk/VkCommon.h"
#include "Engine/Shader/Data/Gpu/GpuShaderData.h"
#include <vector>
#include <string>
#include <cstdint>

namespace Syn 
{
    class SYN_API IShaderReflector {
    public:
        virtual ~IShaderReflector() = default;
        virtual ShaderResources Reflect(const std::vector<uint32_t>& bytecode, VkShaderStageFlagBits stage, const std::string& shaderName) = 0;
    };
}