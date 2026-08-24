// Copyright (C) 2026 Tamás Péter
// This file is part of SynapseEngine.
//
// SynapseEngine is free software: you can redistribute it and/or modify
// it under the terms of the GNU Affero General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// SynapseEngine is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License
// along with SynapseEngine. If not, see <https://www.gnu.org/licenses/>.

#include "DefaultGpuImageUploader.h"
#include "Engine/Vk/Buffer/BufferFactory.h"
#include "Engine/Vk/Buffer/BufferUtils.h"
#include "Engine/Vk/Image/ImageUtils.h"
#include <cmath>
#include <algorithm>

namespace Syn
{
    ImageUploadResult DefaultGpuImageUploader::Upload(const GpuImage& data, VkCommandBuffer cmd, Vk::GpuUploader* uploader)
    {
        ImageUploadResult result;

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
        imgConfig.usage = VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;

        if (data.isGpuGenerated) {
            imgConfig.usage |= VK_IMAGE_USAGE_STORAGE_BIT;
        }

        result.texture = std::make_shared<Vk::Image>(imgConfig);

        if (data.isGpuGenerated)
        {
            result.texture->TransitionLayout(
                cmd,
                VK_IMAGE_LAYOUT_GENERAL,
                VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT,
                VK_ACCESS_2_SHADER_WRITE_BIT,
                true
            );

            if (data.gpuGeneratorCallback) {
                data.gpuGeneratorCallback(cmd, *result.texture);
            }

            result.texture->TransitionLayout(
                cmd,
                VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
                VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT | VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT,
                VK_ACCESS_2_SHADER_READ_BIT,
                false
            );

            result.texture->OverrideInternalState(
                VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
                VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT,
                VK_ACCESS_2_SHADER_READ_BIT
            );

            result.requiresGraphicsQueue = true;
        }
        else
        {
            //Staging Buffer
            size_t byteSize = data.pixels.size();
            result.stagingBuffer = Vk::BufferFactory::CreateStaging(byteSize);
            result.stagingBuffer->Write(data.pixels.data(), byteSize, 0);

            result.texture->TransitionLayout(
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
                    copyInfo.dstImage = result.texture->Handle();

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
                copyInfo.dstImage = result.texture->Handle();

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
                    result.texture->Handle(),
                    data.format,
                    data.width,
                    data.height,
                    targetMipLevels
                );

                result.texture->OverrideInternalState(
                    VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
                    VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT,
                    VK_ACCESS_2_SHADER_READ_BIT
                );

                result.requiresGraphicsQueue = true;
            }
            else {
                uploader->RegisterImageTransfer({
                    .image = result.texture->Handle(),
                    .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                    .mipLevels = targetMipLevels
                    });

                result.texture->OverrideInternalState(
                    VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
                    VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT,
                    VK_ACCESS_2_SHADER_READ_BIT
                );

                result.requiresGraphicsQueue = false;
            }
        }

        return result;
    }
}