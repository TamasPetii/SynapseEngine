#pragma once
#include "Engine/SynApi.h"
#include "Engine/Video/Data/Cooked/CookedVideoPacket.h"

namespace Syn
{
    class SYN_API IVideoProcessor
    {
    public:
        virtual ~IVideoProcessor() = default;
        virtual void Process(CookedVideoPacket& cookedPacket) = 0;
    };
}