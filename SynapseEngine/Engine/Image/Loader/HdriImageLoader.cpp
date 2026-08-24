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

#include "HdriImageLoader.h"
#include <stb_image.h>
#include <iostream>
#include <string>
#include <cstring>
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
            rawImage.format = VK_FORMAT_R32_SFLOAT;
        }
        else if (desiredChannels == 2) {
            rawImage.format = VK_FORMAT_R32G32_SFLOAT;
        }
        else {
            rawImage.format = VK_FORMAT_R32G32B32A32_SFLOAT;
        }

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