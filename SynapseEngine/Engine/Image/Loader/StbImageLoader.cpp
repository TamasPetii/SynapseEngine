#include "StbImageLoader.h"
#include <stb_image.h>
#include <iostream>
#include "Engine/Logger/SynLog.h"

namespace Syn
{
    std::optional<RawImage> StbImageLoader::LoadFile(const std::filesystem::path& path)
    {
        int width, height, originalChannels;

        if (!stbi_info(path.string().c_str(), &width, &height, &originalChannels)) {
			Error("Failed to read image info: {} - {}", path.string(), stbi_failure_reason());
            return std::nullopt;
        }

        int desiredChannels = originalChannels;
        if (originalChannels == 3) {
            desiredChannels = STBI_rgb_alpha;
        }

        stbi_uc* data = stbi_load(path.string().c_str(), &width, &height, nullptr, desiredChannels);

        if (!data) {
			Error("Failed to load image: {} - {}", path.string(), stbi_failure_reason());
            return std::nullopt;
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
        rawImage.pixels.assign(data, data + imageSize);

        stbi_image_free(data);

        return rawImage;
    }

    std::vector<std::string> StbImageLoader::GetSupportedExtensions() const
    {
        return { ".png", ".jpg", ".jpeg", ".bmp", ".tga" };
    }
}