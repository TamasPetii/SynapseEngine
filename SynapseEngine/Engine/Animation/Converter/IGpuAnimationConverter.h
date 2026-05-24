#pragma once
#include "Engine/SynApi.h"
#include "Engine/Animation/Data/Gpu/GpuBatchedAnimation.h"
#include "Engine/Animation/Data/Cooked/CookedAnimation.h"
#include "Engine/Mesh/Data/Cpu/CpuModelData.h"

namespace Syn
{
    class SYN_API IGpuAnimationConverter
    {
    public:
        virtual ~IGpuAnimationConverter() = default;
        virtual GpuBatchedAnimation Convert(const CookedAnimation& cookedAnimation, const CpuModelData& baseModel) const = 0;
    };
}