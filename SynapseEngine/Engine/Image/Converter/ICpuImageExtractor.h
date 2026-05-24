#pragma once
#include "Engine/SynApi.h"
#include "Engine/Image/Data/Gpu/GpuImage.h"
#include "Engine/Image/Data/Cpu/CpuTextureData.h"

namespace Syn
{

    class SYN_API ICpuImageExtractor
    {
    public:
        virtual ~ICpuImageExtractor() = default;
        virtual void Extract(const GpuImage& gpuData, CpuTextureData& outCpuData) const = 0;
    };
}