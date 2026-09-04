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

#include "DefaultGpuImageDownloader.h"
#include "Engine/Vk/Buffer/BufferFactory.h"
#include "Engine/Vk/Image/ImageUtils.h"

namespace Syn
{
    ImageDownloadResult DefaultGpuImageDownloader::Download(Vk::Image& image, VkCommandBuffer cmd)
    {
        ImageDownloadResult result;
        auto config = image.GetConfig();

        size_t bpp = Vk::ImageUtils::GetBytesPerPixel(config.format);
        result.sizeInBytes = config.width * config.height * config.depth * bpp;
        result.stagingBuffer = Vk::BufferFactory::CreateStaging(result.sizeInBytes);

        image.TransitionLayout(
            cmd,
            VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
            VK_PIPELINE_STAGE_2_TRANSFER_BIT,
            VK_ACCESS_2_TRANSFER_READ_BIT,
            false
        );

        Vk::ImageToBufferCopyInfo copyInfo{};
        copyInfo.srcImage = image.Handle();
        copyInfo.dstBuffer = result.stagingBuffer->Handle();
        copyInfo.extent = { config.width, config.height, config.depth };
        copyInfo.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;

        Vk::ImageUtils::CopyImageToBuffer(cmd, copyInfo);

        image.TransitionLayout(
            cmd,
            VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
            VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT | VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT,
            VK_ACCESS_2_SHADER_READ_BIT,
            false
        );

        return result;
    }
}