#pragma once
#include "Engine/SynApi.h"
#include "Engine/Video/Data/Raw/RawVideoPacket.h"
#include "Engine/Video/Data/Common/VideoInfo.h"
#include <optional>

namespace Syn
{
    class SYN_API IVideoSource
    {
    public:
        virtual ~IVideoSource() = default;
        virtual VideoInfo GetInfo() const = 0;
        virtual std::optional<RawVideoPacket> ReadNextPacket() = 0;
        virtual void Reset() = 0;
    };
}