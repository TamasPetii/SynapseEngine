#include "CpuPixelVideoUploader.h"
#include "Engine/Vk/Buffer/BufferFactory.h"
#include "Engine/Vk/Buffer/BufferUtils.h"

namespace Syn
{
    CpuPixelVideoUploader::CpuPixelVideoUploader(uint32_t width, uint32_t height)
        : _width(width), _height(height) {}

    VideoUploadResult CpuPixelVideoUploader::Upload(const GpuVideoPacket& data, VkCommandBuffer cmd)
    {
        VideoUploadResult result;
        result.isFrameReady = false;

        if (data.bitstreamData.empty()) {
            return result;
        }

        if (!_texture) {
            Vk::ImageConfig imgConfig{};
            imgConfig.width = _width;
            imgConfig.height = _height;
            imgConfig.depth = 1;
            imgConfig.format = data.format;
            imgConfig.mipLevels = 1;
            imgConfig.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;

            _texture = std::make_shared<Vk::Image>(imgConfig);
        }

        size_t byteSize = data.bitstreamData.size();
        result.bitstreamBuffer = Vk::BufferFactory::CreateStaging(byteSize);
        result.bitstreamBuffer->Write(data.bitstreamData.data(), byteSize, 0);

        _texture->TransitionLayout(
            cmd,
            VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
            VK_PIPELINE_STAGE_2_TRANSFER_BIT,
            VK_ACCESS_2_TRANSFER_WRITE_BIT,
            true
        );

        Vk::BufferToImageCopyInfo copyInfo{};
        copyInfo.srcBuffer = result.bitstreamBuffer->Handle();
        copyInfo.dstImage = _texture->Handle();
        copyInfo.width = _width;
        copyInfo.height = _height;
        copyInfo.depth = 1;
        copyInfo.bufferOffset = 0;
        copyInfo.mipLevel = 0;
        copyInfo.baseArrayLayer = 0;
        copyInfo.layerCount = 1;
        copyInfo.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        copyInfo.dstImageLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;

        Vk::BufferUtils::CopyBufferToImage(cmd, copyInfo);

        _texture->TransitionLayout(
            cmd,
            VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
            VK_PIPELINE_STAGE_2_BOTTOM_OF_PIPE_BIT,
            VK_ACCESS_2_NONE,
            false
        );

        _texture->OverrideInternalState(
            VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
            VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT,
            VK_ACCESS_2_SHADER_READ_BIT
        );

        result.texture = _texture;
        result.isFrameReady = true;
        return result;
    }
}