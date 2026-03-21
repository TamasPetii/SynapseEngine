#pragma once
#include "IGpuAnimationConverter.h"

namespace Syn
{
    class SYN_API DefaultGpuAnimationConverter : public IGpuAnimationConverter
    {
    public:
        DefaultGpuAnimationConverter() = default;
        ~DefaultGpuAnimationConverter() override = default;

        GpuBatchedAnimation Convert(const CookedAnimation& cookedAnimation, const CookedModel& baseModel) const override;
    };
}