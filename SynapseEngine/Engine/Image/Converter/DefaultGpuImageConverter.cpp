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

#include "DefaultGpuImageConverter.h"

namespace Syn
{
    GpuImage DefaultGpuImageConverter::Convert(const CookedImage& cookedImage)
    {
        GpuImage gpuData{};

        gpuData.width = cookedImage.width;
        gpuData.height = cookedImage.height;
        gpuData.depth = cookedImage.depth;
        gpuData.format = cookedImage.format;
        gpuData.mipLevels = cookedImage.mipLevels;
        gpuData.autoGenerateMipmaps = cookedImage.autoGenerateMipmaps;

        gpuData.pixels = cookedImage.pixels;
        gpuData.mipData = cookedImage.mipData;
		gpuData.autoGenerateMipmaps = cookedImage.autoGenerateMipmaps;
		gpuData.isCompressed = cookedImage.isCompressed;

        gpuData.isGpuGenerated = cookedImage.isGpuGenerated;
        gpuData.gpuGeneratorCallback = cookedImage.gpuGeneratorCallback;
		gpuData.autoCache = cookedImage.autoCache;

        return gpuData;
    }
}