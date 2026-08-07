#pragma once
#include "Engine/SynApi.h"
#include "IGpuVideoUploader.h"

namespace Syn
{
    class SYN_API VulkanGpuVideoUploader : public IGpuVideoUploader
    {
    public:
        VulkanGpuVideoUploader(uint32_t width, uint32_t height);
        ~VulkanGpuVideoUploader() override;

        VideoUploadResult Upload(const GpuVideoPacket& data, VkCommandBuffer cmd) override;
    private:
        uint32_t _width;
        uint32_t _height;
        VkVideoSessionKHR _videoSession = VK_NULL_HANDLE;
        VkVideoSessionParametersKHR _sessionParams = VK_NULL_HANDLE;
        VkSamplerYcbcrConversion _ycbcrConversion = VK_NULL_HANDLE;
        std::shared_ptr<Vk::Image> _texture;
    };
}