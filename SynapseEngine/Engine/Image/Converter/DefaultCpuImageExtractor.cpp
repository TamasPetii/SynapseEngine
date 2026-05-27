#include "DefaultCpuImageExtractor.h"

namespace Syn
{
    void DefaultCpuImageExtractor::Extract(const GpuImage& gpuData, CpuTextureData& outCpuData) const
    {
        outCpuData.width = gpuData.width;
        outCpuData.height = gpuData.height;
        outCpuData.depth = gpuData.depth;
        outCpuData.mipLevels = gpuData.mipLevels;
        outCpuData.format = gpuData.format;
        outCpuData.isCompressed = gpuData.isCompressed;
    }
}