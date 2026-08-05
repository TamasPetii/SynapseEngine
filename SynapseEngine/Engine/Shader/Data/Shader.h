#pragma once
#include "Engine/SynApi.h"
#include "Engine/Vk/VkCommon.h"
#include "Cpu/CpuShaderData.h"
#include "Gpu/GpuShaderData.h"
#include "Cooked/CookedShader.h"
#include <memory>

namespace Syn {
    struct SYN_API Shader 
    {
        CpuShaderData cpuData;
        VkShaderEXT handle = VK_NULL_HANDLE;

        std::unique_ptr<CookedShader> transientCpuData;
        std::unique_ptr<GpuShaderData> transientGpuData;
    };
}