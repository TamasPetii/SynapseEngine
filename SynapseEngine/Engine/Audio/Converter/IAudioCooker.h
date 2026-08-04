#pragma once
#include "Engine/SynApi.h"
#include "Engine/Audio/Data/Cooked/CookedAudio.h"
#include "Engine/Audio/Data/Raw/RawAudio.h"

namespace Syn
{
    class SYN_API IAudioCooker
    {
    public:
        virtual ~IAudioCooker() = default;
        virtual CookedAudio Cook(const RawAudio& rawAudio) = 0;
    };
}