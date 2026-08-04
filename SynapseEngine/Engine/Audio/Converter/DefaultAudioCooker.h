#pragma once
#include "Engine/SynApi.h"
#include "IAudioCooker.h"

namespace Syn
{
    class SYN_API DefaultAudioCooker : public IAudioCooker
    {
    public:
        DefaultAudioCooker() = default;
        ~DefaultAudioCooker() override = default;

        CookedAudio Cook(const RawAudio& rawAudio) override;
    };
}