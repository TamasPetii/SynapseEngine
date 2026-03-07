#pragma once
#include "Engine/SynApi.h"
#include "Engine/Image/Data/Cooked/CookedImage.h"
#include "Engine/Image/Data/Gpu/GpuImage.h"

namespace Syn
{
    class SYN_API IGpuImageConverter
    {
    public:
        virtual ~IGpuImageConverter() = default;
        virtual GpuImage Convert(const CookedImage& cookedImage) = 0;
    };
}