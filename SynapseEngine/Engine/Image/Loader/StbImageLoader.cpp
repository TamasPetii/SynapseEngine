#include "StbImageLoader.h"
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
#include <iostream>
#include "Engine/Logger/SynLog.h"

namespace Syn
{
    std::optional<RawImage> StbImageLoader::LoadFile(const std::filesystem::path& path) {
        int width, height, originalChannels;

        stbi_uc* data = stbi_load(path.string().c_str(), &width, &height, &originalChannels, STBI_default);

        if (!data) {
            Error("Failed to load image: {} - {}", path.string(), stbi_failure_reason());
            return std::nullopt;
        }

        int desiredChannels = originalChannels;
        if (originalChannels == 3) {
            desiredChannels = 4;
        }

        RawImage rawImage{};
        rawImage.width = static_cast<uint32_t>(width);
        rawImage.height = static_cast<uint32_t>(height);
        rawImage.depth = 1;
        rawImage.mipLevels = 1;
        rawImage.isCompressed = false;

        if (desiredChannels == 1) {
            rawImage.format = VK_FORMAT_R8_UNORM;
        }
        else if (desiredChannels == 2) {
            rawImage.format = VK_FORMAT_R8G8_UNORM;
        }
        else {
            rawImage.format = VK_FORMAT_R8G8B8A8_UNORM;
        }

        size_t imageSize = static_cast<size_t>(width * height * desiredChannels);

        if (originalChannels == 3) {
            rawImage.pixels.resize(imageSize);
            for (size_t i = 0; i < static_cast<size_t>(width * height); ++i) {
                rawImage.pixels[i * 4 + 0] = data[i * 3 + 0];
                rawImage.pixels[i * 4 + 1] = data[i * 3 + 1];
                rawImage.pixels[i * 4 + 2] = data[i * 3 + 2];
                rawImage.pixels[i * 4 + 3] = 255;
            }
        }
        else {
            rawImage.pixels.assign(data, data + imageSize);
        }

        stbi_image_free(data);

        return rawImage;
    }

    std::vector<std::string> StbImageLoader::GetSupportedExtensions() const
    {
        return { ".png", ".jpg", ".jpeg", ".bmp", ".tga" };
    }
}