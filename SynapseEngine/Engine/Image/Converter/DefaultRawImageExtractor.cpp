#include "DefaultRawImageExtractor.h"

namespace Syn {
    RawImage DefaultRawImageExtractor::Extract(const ImageDownloadResult& downloadData, const Vk::Image& targetImage) {
        RawImage rawImage{};
        auto config = targetImage.GetConfig();

        rawImage.width = config.width;
        rawImage.height = config.height;
        rawImage.depth = config.depth;
        rawImage.mipLevels = config.mipLevels;
        rawImage.format = config.format;
        rawImage.isCompressed = false;
        rawImage.isGpuGenerated = false;

        if (downloadData.stagingBuffer && downloadData.sizeInBytes > 0) {
            rawImage.pixels.resize(downloadData.sizeInBytes);
            downloadData.stagingBuffer->Read(rawImage.pixels.data(), downloadData.sizeInBytes, 0);
        }

        return rawImage;
    }
}