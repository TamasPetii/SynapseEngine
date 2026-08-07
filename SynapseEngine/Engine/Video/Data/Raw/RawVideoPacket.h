#pragma once
#include "Engine/SynApi.h"
#include <vector>
#include <cstdint>

namespace Syn
{
    struct SYN_API RawVideoPacket {
        std::vector<uint8_t> data;
        int64_t pts = 0;
        int64_t dts = 0;
        bool isKeyFrame = false;
    };
}