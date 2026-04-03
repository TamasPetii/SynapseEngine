#pragma once
#include "Engine/SynApi.h"
#include "Engine/Mesh/Data/Gpu/GpuBatchedModel.h"
#include "Engine/Mesh/Data/Cooked/CookedModel.h"

namespace Syn
{
    class SYN_API IGpuModelConverter
    {
    public:
        virtual ~IGpuModelConverter() = default;
        virtual GpuBatchedModel Convert(const CookedModel& cookedModel) const = 0;
    };
}