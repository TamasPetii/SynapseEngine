#pragma once
#include "Engine/SynApi.h"
#include "Engine/Vk/Image/Image.h"
#include "Engine/Vk/Buffer/Buffer.h"
#include "Engine/Video/Data/Cooked/CookedVideoPacket.h"
#include "Engine/Video/Data/Gpu/GpuVideoPacket.h"
#include "Engine/Video/Data/Common/VideoInfo.h"

namespace Syn
{
    struct SYN_API Video {
        VideoInfo info;
        std::shared_ptr<Vk::Image> image;

        std::unique_ptr<CookedVideoPacket> transientCpuData;
        std::unique_ptr<GpuVideoPacket> transientGpuData;
    };
}