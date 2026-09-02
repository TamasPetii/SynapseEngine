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

#include "StbImageWriter.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb_image_write.h>
#include "Engine/Logger/SynLog.h"
#include <algorithm>
#include "Engine/Vk/Image/ImageUtils.h"

namespace Syn
{
    bool StbImageWriter::WriteFile(const std::filesystem::path& path, const RawImage& image)
    {
        if (image.pixels.empty()) {
            Error("StbImageWriter: Failed to save, pixel data is empty ({})", path.string());
            return false;
        }

        std::string ext = path.extension().string();
        std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);

        int channels = Vk::ImageUtils::GetChannelCount(image.format);
        if (channels == 0) {
            Error("StbImageWriter: Unsupported Vulkan format for saving ({})", path.string());
            return false;
        }

        bool isFloat = Vk::ImageUtils::IsFloatFormat(image.format);
        int result = 0;

        if (ext == ".png" && !isFloat) {
            result = stbi_write_png(path.string().c_str(), image.width, image.height, channels, image.pixels.data(), image.width * channels);
        }
        else if (ext == ".hdr" && isFloat) {
            result = stbi_write_hdr(path.string().c_str(), image.width, image.height, channels, reinterpret_cast<const float*>(image.pixels.data()));
        }
        else if (ext == ".png" && isFloat) {
            std::vector<uint8_t> converted(image.width * image.height * channels);
            const float* floatData = reinterpret_cast<const float*>(image.pixels.data());

            for (size_t i = 0; i < converted.size(); ++i) {
                converted[i] = static_cast<uint8_t>(std::clamp(floatData[i] * 255.0f, 0.0f, 255.0f));
            }
            result = stbi_write_png(path.string().c_str(), image.width, image.height, channels, converted.data(), image.width * channels);
        }
        else {
            Error("StbImageWriter: Unsupported extension or format mismatch (Ext: {}, Float: {})", ext, isFloat);
            return false;
        }

        if (result == 0) {
            Error("StbImageWriter: STB error while writing file: {}", path.string());
            return false;
        }

        return true;
    }

    std::vector<std::string> StbImageWriter::GetSupportedExtensions() const
    {
        return { ".png", ".jpg", ".jpeg", ".bmp", ".tga", ".hdr" };
    }
}