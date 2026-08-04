#pragma once
#include "Engine/SynApi.h"
#include "Engine/Audio/Data/Cooked/CookedAudio.h"

namespace Syn
{
    class SYN_API IAudioProcessor
    {
    public:
        virtual ~IAudioProcessor() = default;
        virtual void Process(CookedAudio& cookedAudio) = 0;
    };
}