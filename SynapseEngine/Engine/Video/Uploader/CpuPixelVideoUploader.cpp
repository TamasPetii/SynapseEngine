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

#include "CpuPixelVideoUploader.h"
#include "Engine/Vk/Buffer/BufferFactory.h"
#include "Engine/Vk/Buffer/BufferUtils.h"

namespace Syn
{
    CpuPixelVideoUploader::CpuPixelVideoUploader(uint32_t width, uint32_t height, uint32_t bufferCount)
        : _width(width), _height(height)
    {
        _textures.resize(bufferCount);
    }

    VideoUploadResult CpuPixelVideoUploader::Upload(const GpuVideoPacket& data, VkCommandBuffer cmd, Vk::GpuUploader* uploader)
    {
        VideoUploadResult result;
        result.isFrameReady = false;

        if (data.bitstreamData.empty()) {
            return result;
        }

        if (!_textures[0]) {
            Vk::ImageConfig imgConfig{};
            imgConfig.width = _width;
            imgConfig.height = _height;
            imgConfig.depth = 1;
            imgConfig.format = data.format;
            imgConfig.mipLevels = 1;
            imgConfig.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;

            for (auto& tex : _textures) {
                tex = std::make_shared<Vk::Image>(imgConfig);
            }
        }

        uint32_t currentIndex = _frameIndex % _textures.size();
        _frameIndex++;
        auto currentTexture = _textures[currentIndex];

        size_t byteSize = data.bitstreamData.size();
        result.bitstreamBuffer = Vk::BufferFactory::CreateStaging(byteSize);
        result.bitstreamBuffer->Write(data.bitstreamData.data(), byteSize, 0);

        currentTexture->TransitionLayout(
            cmd,
            VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
            VK_PIPELINE_STAGE_2_TRANSFER_BIT,
            VK_ACCESS_2_TRANSFER_WRITE_BIT,
            true
        );

        Vk::BufferToImageCopyInfo copyInfo{};
        copyInfo.srcBuffer = result.bitstreamBuffer->Handle();
        copyInfo.dstImage = currentTexture->Handle();
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

        uploader->RegisterImageTransfer({
            .image = currentTexture->Handle(),
            .mipLevels = 1
        });

        currentTexture->OverrideInternalState(
            VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
            VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT,
            VK_ACCESS_2_SHADER_READ_BIT
        );

        result.texture = currentTexture;
        result.isFrameReady = true;
        return result;
    }
}