#pragma once
#include "Engine/SynApi.h"
#include "Engine/Audio/Data/Raw/RawAudio.h"
#include <optional>

namespace Syn
{
    class SYN_API IAudioSource
    {
    public:
        virtual ~IAudioSource() = default;
        virtual std::optional<RawAudio> Produce() = 0;
    };
}