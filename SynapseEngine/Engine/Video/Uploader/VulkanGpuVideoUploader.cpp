#include "VulkanGpuVideoUploader.h"
#include "Engine/Vk/Buffer/BufferFactory.h"
#include "Engine/Vk/Buffer/BufferUtils.h"

namespace Syn
{
    VulkanGpuVideoUploader::VulkanGpuVideoUploader(uint32_t width, uint32_t height)
        : _width(width), _height(height) {}

    VideoUploadResult VulkanGpuVideoUploader::Upload(const GpuVideoPacket& data, VkCommandBuffer cmd)
    {
        VideoUploadResult result;
        result.isFrameReady = false;

        if (data.bitstreamData.empty()) {
            return result;
        }

        size_t byteSize = data.bitstreamData.size();
        result.bitstreamBuffer = Vk::BufferFactory::CreateStaging(byteSize);
        result.bitstreamBuffer->Write(data.bitstreamData.data(), byteSize, 0);

        if (!result.texture) {
            Vk::ImageConfig imgConfig{};
            imgConfig.width = _width;
            imgConfig.height = _height;
            imgConfig.depth = 1;
            imgConfig.format = data.format;
            imgConfig.mipLevels = 1;
            imgConfig.usage = VK_IMAGE_USAGE_VIDEO_DECODE_DST_BIT_KHR | VK_IMAGE_USAGE_SAMPLED_BIT;

            result.texture = std::make_shared<Vk::Image>(imgConfig);
        }

        result.texture->TransitionLayout(
            cmd,
            VK_IMAGE_LAYOUT_VIDEO_DECODE_DST_KHR,
            VK_PIPELINE_STAGE_2_VIDEO_DECODE_BIT_KHR,
            VK_ACCESS_2_VIDEO_DECODE_WRITE_BIT_KHR,
            true
        );

        if (_videoSession != VK_NULL_HANDLE) {
            VkVideoBeginCodingInfoKHR beginInfo{ VK_STRUCTURE_TYPE_VIDEO_BEGIN_CODING_INFO_KHR };
            beginInfo.videoSession = _videoSession;
            beginInfo.videoSessionParameters = _sessionParams;

            vkCmdBeginVideoCodingKHR(cmd, &beginInfo);

            VkVideoDecodeInfoKHR decodeInfo{ VK_STRUCTURE_TYPE_VIDEO_DECODE_INFO_KHR };
            decodeInfo.srcBuffer = result.bitstreamBuffer->Handle();
            decodeInfo.srcBufferOffset = 0;
            decodeInfo.srcBufferRange = byteSize;

            VkVideoPictureResourceInfoKHR dstPictureResource{ VK_STRUCTURE_TYPE_VIDEO_PICTURE_RESOURCE_INFO_KHR };
            dstPictureResource.imageViewBinding = result.texture->GetView();
            decodeInfo.dstPictureResource = dstPictureResource;

            vkCmdDecodeVideoKHR(cmd, &decodeInfo);

            VkVideoEndCodingInfoKHR endInfo{ VK_STRUCTURE_TYPE_VIDEO_END_CODING_INFO_KHR };
            vkCmdEndVideoCodingKHR(cmd, &endInfo);
        }

        result.texture->TransitionLayout(
            cmd,
            VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
            VK_PIPELINE_STAGE_2_BOTTOM_OF_PIPE_BIT,
            VK_ACCESS_2_NONE,
            false
        );

        result.texture->OverrideInternalState(
            VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
            VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT,
            VK_ACCESS_2_SHADER_READ_BIT
        );

        result.isFrameReady = true;
        return result;
    }
}