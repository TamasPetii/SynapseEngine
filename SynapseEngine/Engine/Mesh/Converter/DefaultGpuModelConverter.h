#pragma once
#include "IGpuModelConverter.h"

namespace Syn
{
    class SYN_API DefaultGpuModelConverter : public IGpuModelConverter
    {
    public:
        DefaultGpuModelConverter() = default;
        ~DefaultGpuModelConverter() override = default;

        GpuBatchedModel Convert(const CookedModel& cookedModel) const override;
    };
}