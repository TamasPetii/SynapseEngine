#pragma once
#include "Engine/SynApi.h"
#include "Engine/Vk/VkCommon.h"
#include "Engine/Shader/Data/Gpu/GpuShaderData.h"

namespace Syn {
    struct SYN_API ShaderUploadResult {
        VkShaderEXT handle = VK_NULL_HANDLE;
    };

    class SYN_API IGpuShaderUploader {
    public:
        virtual ~IGpuShaderUploader() = default;
        virtual ShaderUploadResult Upload(const GpuShaderData& data) = 0;
    };
}