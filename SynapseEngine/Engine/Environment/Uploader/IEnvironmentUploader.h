#pragma once
#include "Engine/SynApi.h"
#include "Engine/Environment/Data/EnvironmentData.h"
#include "Engine/Vk/Rendering/GpuUploader.h"

namespace Syn {
    struct EnvironmentUploadResult {
        std::shared_ptr<Texture> baseCubemap;
        std::shared_ptr<Texture> irradianceMap;
        std::shared_ptr<Texture> prefilteredMap;
        std::unique_ptr<Vk::Buffer> stagingBuffer;
    };

    class SYN_API IEnvironmentUploader {
    public:
        virtual ~IEnvironmentUploader() = default;
        virtual EnvironmentUploadResult Upload(Environment& data, VkCommandBuffer cmd, Vk::GpuUploader* uploader) = 0;
    };
}