#include "DefaultGpuImageUploader.h"
#include "Engine/Vk/Buffer/BufferFactory.h"
#include "Engine/Vk/Buffer/BufferUtils.h"
#include "Engine/Vk/Image/ImageUtils.h"
#include <cmath>
#include <algorithm>

namespace Syn
{
    ImageUploadResult DefaultGpuImageUploader::Upload(const GpuImage& data, VkCommandBuffer cmd)
    {
        ImageUploadResult result;
        result.texture = std::make_shared<Texture>();

        uint32_t targetMipLevels = data.mipLevels;
        if (data.autoGenerateMipmaps) {
            targetMipLevels = static_cast<uint32_t>(std::floor(std::log2(std::max(data.width, data.height)))) + 1;
        }

        //Image Creation
        Vk::ImageConfig imgConfig{};
        imgConfig.width = data.width;
        imgConfig.height = data.height;
        imgConfig.depth = data.depth;
        imgConfig.format = data.format;
        imgConfig.mipLevels = targetMipLevels;
        imgConfig.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;

        if (data.autoGenerateMipmaps) {
            imgConfig.usage |= VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
        }

        result.texture->image = std::make_shared<Vk::Image>(imgConfig);

        //Staging Buffer
        size_t byteSize = data.pixels.size();
        result.stagingBuffer = Vk::BufferFactory::CreateStaging(byteSize);
        result.stagingBuffer->Write(data.pixels.data(), byteSize, 0);

        result.texture->image->TransitionLayout(
            cmd,
            VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
            VK_PIPELINE_STAGE_2_TRANSFER_BIT,
            VK_ACCESS_2_TRANSFER_WRITE_BIT,
            true
        );

        if (!data.mipData.empty()) {
            for (uint32_t level = 0; level < data.mipLevels; ++level) {
                const auto& mip = data.mipData[level];

                Vk::BufferToImageCopyInfo copyInfo{};
                copyInfo.srcBuffer = result.stagingBuffer->Handle();
                copyInfo.dstImage = result.texture->image->Handle();

                copyInfo.width = mip.width;
                copyInfo.height = mip.height;
                copyInfo.depth = data.depth;

                copyInfo.bufferOffset = mip.offset;

                copyInfo.mipLevel = level;
                copyInfo.baseArrayLayer = 0;
                copyInfo.layerCount = 1;
                copyInfo.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
                copyInfo.dstImageLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;

                Vk::BufferUtils::CopyBufferToImage(cmd, copyInfo);
            }
        }
        else {
            Vk::BufferToImageCopyInfo copyInfo{};
            copyInfo.srcBuffer = result.stagingBuffer->Handle();
            copyInfo.dstImage = result.texture->image->Handle();

            copyInfo.width = data.width;
            copyInfo.height = data.height;
            copyInfo.depth = data.depth;

            copyInfo.bufferOffset = 0;

            copyInfo.mipLevel = 0;
            copyInfo.baseArrayLayer = 0;
            copyInfo.layerCount = 1;
            copyInfo.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            copyInfo.dstImageLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;

            Vk::BufferUtils::CopyBufferToImage(cmd, copyInfo);
        }

        if (data.autoGenerateMipmaps && targetMipLevels > 1) {
            Vk::ImageUtils::GenerateMipMaps(
                cmd,
                result.texture->image->Handle(),
                data.format,
                data.width,
                data.height,
                targetMipLevels
            );

            result.texture->image->OverrideInternalState(
                VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
                VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT,
                VK_ACCESS_2_SHADER_READ_BIT
            );

            result.requiresGraphicsQueue = true;
        }
        else {
            result.texture->image->TransitionLayout(
                cmd,
                VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
                VK_PIPELINE_STAGE_2_ALL_TRANSFER_BIT,
                VK_ACCESS_2_TRANSFER_WRITE_BIT,
                false
            );

            result.texture->image->OverrideInternalState(
                VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
                VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT,
                VK_ACCESS_2_SHADER_READ_BIT
            );

            result.requiresGraphicsQueue = false;
        }

        return result;
    }
}