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