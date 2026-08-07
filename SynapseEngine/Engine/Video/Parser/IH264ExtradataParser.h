#pragma once
#include "Engine/SynApi.h"
#include <vector>
#include <cstdint>
#include <vk_video/vulkan_video_codec_h264std.h>

namespace Syn
{
    class SYN_API IH264ExtradataParser
    {
    public:
        virtual ~IH264ExtradataParser() = default;
        virtual bool Parse(const std::vector<uint8_t>& extradata, StdVideoH264SequenceParameterSet& outSps, StdVideoH264PictureParameterSet& outPps) = 0;
    };
}