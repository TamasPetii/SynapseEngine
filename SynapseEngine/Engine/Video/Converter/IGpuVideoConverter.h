#pragma once
#include "Engine/SynApi.h"
#include "Engine/Video/Data/Cooked/CookedVideoPacket.h"
#include "Engine/Video/Data/Gpu/GpuVideoPacket.h"

namespace Syn
{
    class SYN_API IGpuVideoConverter
    {
    public:
        virtual ~IGpuVideoConverter() = default;
        virtual GpuVideoPacket Convert(const CookedVideoPacket& cookedPacket) = 0;
    };
}