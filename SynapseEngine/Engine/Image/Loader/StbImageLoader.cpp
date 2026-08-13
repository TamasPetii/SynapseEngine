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

        return ProcessData(data, width, height, originalChannels);
    }

    std::optional<RawImage> StbImageLoader::LoadMemory(const std::vector<uint8_t>& data) {
        int width, height, originalChannels;
        stbi_uc* stbiData = stbi_load_from_memory(data.data(), static_cast<int>(data.size()), &width, &height, &originalChannels, STBI_default);

        if (!stbiData) {
            Error("StbImageLoader failed to load image from memory - {}", stbi_failure_reason());
            return std::nullopt;
        }

        return ProcessData(stbiData, width, height, originalChannels);
    }

    std::optional<RawImage> StbImageLoader::ProcessData(stbi_uc* data, int width, int height, int originalChannels) {

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