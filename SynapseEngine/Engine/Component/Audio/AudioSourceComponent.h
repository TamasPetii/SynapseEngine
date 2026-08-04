#pragma once
#include "Engine/SynApi.h"
#include "Engine/Component/Core/Component.h"
#include <cstdint>

namespace Syn
{
    struct SYN_API AudioSourceComponent : public Component
    {
        AudioSourceComponent();

        uint32_t soundIndex;

        bool play;
        bool loop;
        bool isSpatialized;

        float volume;
        float pitch;

        float minDistance;
        float maxDistance;
    };
}