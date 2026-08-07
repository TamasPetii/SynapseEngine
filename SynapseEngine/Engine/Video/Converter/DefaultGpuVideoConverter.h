#pragma once
#include "Engine/SynApi.h"
#include "IGpuVideoConverter.h"

namespace Syn
{
    class SYN_API DefaultGpuVideoConverter : public IGpuVideoConverter
    {
    public:
        DefaultGpuVideoConverter() = default;
        ~DefaultGpuVideoConverter() override = default;

        GpuVideoPacket Convert(const CookedVideoPacket& cookedPacket) override;
    };
}