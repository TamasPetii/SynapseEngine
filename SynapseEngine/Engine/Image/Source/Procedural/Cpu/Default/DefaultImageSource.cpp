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

#include "DefaultImageSource.h"
#include "Engine/Image/ImageNames.h"

namespace Syn
{
    DefaultImageSource::DefaultImageSource()
        : ProceduralImageSource(ImageNames::Default)
    {}

    std::optional<RawImage> DefaultImageSource::Produce()
    {
        RawImage image{};

        image.width = 1;
        image.height = 1;
        image.depth = 1;
        image.mipLevels = 1;
        image.format = VK_FORMAT_R8G8B8A8_UNORM;
        image.isCompressed = false;
        image.pixels = { 255, 255, 255, 255 };

        MipLevelInfo mip0{};
        mip0.width = 1;
        mip0.height = 1;
        mip0.size = 4;
        mip0.offset = 0;

        image.mipData.push_back(mip0);
        return image;
    }
}