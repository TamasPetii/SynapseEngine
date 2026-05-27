#pragma once
#include "ICpuImageExtractor.h"

namespace Syn
{

    class SYN_API DefaultCpuImageExtractor : public ICpuImageExtractor
    {
    public:
        virtual ~DefaultCpuImageExtractor() = default;
        void Extract(const GpuImage& gpuData, CpuTextureData& outCpuData) const override;
    };
}