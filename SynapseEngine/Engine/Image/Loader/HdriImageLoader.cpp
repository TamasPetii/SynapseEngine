#include "HdriImageLoader.h"
#include <stb_image.h>
#include <iostream>
#include "Engine/Logger/SynLog.h"

namespace Syn
{
    std::optional<RawImage> HdriImageLoader::LoadFile(const std::filesystem::path& path) {
        int width, height, originalChannels;

        float* data = stbi_loadf(path.string().c_str(), &width, &height, &originalChannels, STBI_default);

        if (!data) {
            Error("Failed to load HDR image: {} - {}", path.string(), stbi_failure_reason());
            return std::nullopt;
        }

        return ProcessData(data, width, height, originalChannels);
    }

    std::optional<RawImage> HdriImageLoader::LoadMemory(const std::vector<uint8_t>& data) {
        int width, height, originalChannels;

        float* stbiData = stbi_loadf_from_memory(data.data(), static_cast<int>(data.size()), &width, &height, &originalChannels, STBI_default);

        if (!stbiData) {
            Error("HdriImageLoader failed to load HDR image from memory - {}", stbi_failure_reason());
            return std::nullopt;
        }

        return ProcessData(stbiData, width, height, originalChannels);
    }

    std::optional<RawImage> HdriImageLoader::ProcessData(float* data, int width, int height, int originalChannels) {

        int desiredChannels = 4;

        RawImage rawImage{};
        rawImage.width = static_cast<uint32_t>(width);
        rawImage.height = static_cast<uint32_t>(height);
        rawImage.depth = 1;
        rawImage.mipLevels = 1;
        rawImage.isCompressed = false;
        rawImage.format = VK_FORMAT_R32G32B32A32_SFLOAT;

        size_t numPixels = static_cast<size_t>(width * height);
        size_t imageSizeInBytes = numPixels * desiredChannels * sizeof(float);

        rawImage.pixels.resize(imageSizeInBytes);
        float* destPixels = reinterpret_cast<float*>(rawImage.pixels.data());

        if (originalChannels == 3) {
            for (size_t i = 0; i < numPixels; ++i) {
                destPixels[i * 4 + 0] = data[i * 3 + 0];
                destPixels[i * 4 + 1] = data[i * 3 + 1];
                destPixels[i * 4 + 2] = data[i * 3 + 2];
                destPixels[i * 4 + 3] = 1.0f;
            }
        }
        else {
            memcpy(destPixels, data, imageSizeInBytes);
        }

        stbi_image_free(data);
        return rawImage;
    }

    std::vector<std::string> HdriImageLoader::GetSupportedExtensions() const
    {
        return { ".hdr" };
    }
}