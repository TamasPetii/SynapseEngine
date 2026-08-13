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

#include "DefaultImageCooker.h"

namespace Syn
{
    CookedImage DefaultImageCooker::Cook(const RawImage& rawImage)
    {
        CookedImage cookedImage{};

        cookedImage.width = rawImage.width;
        cookedImage.height = rawImage.height;
        cookedImage.depth = rawImage.depth;
        cookedImage.format = rawImage.format;
        cookedImage.mipLevels = rawImage.mipLevels;
        cookedImage.isCompressed = rawImage.isCompressed;
        cookedImage.pixels = rawImage.pixels;
        cookedImage.mipData = rawImage.mipData;

        if (!cookedImage.isCompressed && cookedImage.mipLevels == 1) {
            cookedImage.autoGenerateMipmaps = true;
        }
        else {
            cookedImage.autoGenerateMipmaps = false;
        }

        return cookedImage;
    }
}