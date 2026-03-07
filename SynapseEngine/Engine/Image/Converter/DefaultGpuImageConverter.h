#pragma once
#include "Engine/SynApi.h"
#include "IGpuImageConverter.h"

namespace Syn
{
    class SYN_API DefaultGpuImageConverter : public IGpuImageConverter
    {
    public:
        DefaultGpuImageConverter() = default;
        ~DefaultGpuImageConverter() override = default;

        GpuImage Convert(const CookedImage& cookedImage) override;
    };
}