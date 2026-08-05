#pragma once
#include "Engine/SynApi.h"
#include "IGpuShaderUploader.h"

namespace Syn {
    class SYN_API DefaultGpuShaderUploader : public IGpuShaderUploader {
    public:
        DefaultGpuShaderUploader() = default;
        ~DefaultGpuShaderUploader() override = default;

        ShaderUploadResult Upload(const GpuShaderData& data) override;
    };
}