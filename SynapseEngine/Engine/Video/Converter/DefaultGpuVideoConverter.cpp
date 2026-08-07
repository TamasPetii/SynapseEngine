#include "DefaultGpuVideoConverter.h"

namespace Syn
{
    GpuVideoPacket DefaultGpuVideoConverter::Convert(const CookedVideoPacket& cookedPacket)
    {
        GpuVideoPacket gpuPacket{};

        gpuPacket.bitstreamData = cookedPacket.data;
        gpuPacket.pts = cookedPacket.pts;
        gpuPacket.dts = cookedPacket.dts;
        gpuPacket.isKeyFrame = cookedPacket.isKeyFrame;
        gpuPacket.format = cookedPacket.format;

        return gpuPacket;
    }
}