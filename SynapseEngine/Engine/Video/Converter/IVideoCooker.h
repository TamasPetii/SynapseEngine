#pragma once
#include "Engine/SynApi.h"
#include "Engine/Video/Data/Cooked/CookedVideoPacket.h"
#include "Engine/Video/Data/Raw/RawVideoPacket.h"

namespace Syn
{
    class SYN_API IVideoCooker
    {
    public:
        virtual ~IVideoCooker() = default;
        virtual CookedVideoPacket Cook(const RawVideoPacket& rawPacket) = 0;
    };
}