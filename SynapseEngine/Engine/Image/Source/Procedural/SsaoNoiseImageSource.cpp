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

#include "SsaoNoiseImageSource.h"
#include "Engine/Image/ImageNames.h"
#include <random>
#include <cstring>

namespace Syn
{
    SsaoNoiseImageSource::SsaoNoiseImageSource() : 
        ProceduralImageSource(ImageNames::SsaoNoiseTexture) {}

    std::optional<RawImage> SsaoNoiseImageSource::Produce()
    {
        RawImage image{};
        image.width = 4;
        image.height = 4;
        image.depth = 1;
        image.mipLevels = 1;
        image.format = VK_FORMAT_R16G16B16A16_SFLOAT;
        image.isCompressed = false;

        std::mt19937 generator;
        std::uniform_real_distribution<float> dist(0.0f, 1.0f);

        std::vector<uint16_t> pixelData(16 * 4);
        for (int i = 0; i < 16; ++i) {
            pixelData[i * 4 + 0] = static_cast<uint16_t>(dist(generator) * 65535.0f);
            pixelData[i * 4 + 1] = static_cast<uint16_t>(dist(generator) * 65535.0f);
            pixelData[i * 4 + 2] = 0;
            pixelData[i * 4 + 3] = 1.0f;
        }
       
		image.pixels.resize(pixelData.size() * sizeof(uint16_t));
        memcpy(image.pixels.data(), pixelData.data(), image.pixels.size());

        MipLevelInfo mip0{};
        mip0.width = 1;
        mip0.height = 1;
        mip0.size = 4;
        mip0.offset = 0;

        image.mipData.push_back(mip0);
        return image;
    }
}