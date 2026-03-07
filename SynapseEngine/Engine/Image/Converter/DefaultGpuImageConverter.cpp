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

        return gpuData;
    }
}