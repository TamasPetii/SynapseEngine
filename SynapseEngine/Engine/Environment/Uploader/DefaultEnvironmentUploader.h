#pragma once
#include "Engine/SynApi.h"
#include "IEnvironmentUploader.h"
#include "Engine/Environment/Bake/EnvironmentBakerPipeline.h"

namespace Syn {
    class SYN_API DefaultEnvironmentUploader : public IEnvironmentUploader {
    public:
        DefaultEnvironmentUploader();
        EnvironmentUploadResult Upload(Environment& data, VkCommandBuffer cmd, Vk::GpuUploader* uploader) override;

    private:
        EnvironmentBakerPipeline _pipeline;
    };
}