#pragma once
#include "Engine/SynApi.h"
#include "IGpuVideoUploader.h"
#include "Engine/Video/Parser/IH264ExtradataParser.h"
#include <vector>
#include <memory>

namespace Syn
{
    class SYN_API VulkanGpuVideoUploader : public IGpuVideoUploader
    {
    public:
        VulkanGpuVideoUploader(
            uint32_t width,
            uint32_t height,
            uint32_t bufferCount,
            const std::vector<uint8_t>& extradata,
            std::shared_ptr<IH264ExtradataParser> parser
        );
        ~VulkanGpuVideoUploader() override;

        VideoUploadResult Upload(const GpuVideoPacket& data, VkCommandBuffer cmd) override;
    private:
        uint32_t _width;
        uint32_t _height;
        std::vector<uint8_t> _extradata;
        std::shared_ptr<IH264ExtradataParser> _parser;

        VkVideoSessionKHR _videoSession = VK_NULL_HANDLE;
        VkVideoSessionParametersKHR _sessionParams = VK_NULL_HANDLE;
        VkSamplerYcbcrConversion _ycbcrConversion = VK_NULL_HANDLE;
        std::vector<VkDeviceMemory> _sessionMemories;

        std::vector<std::shared_ptr<Vk::Image>> _textures;
        uint32_t _frameIndex = 0;
    };
}