#include "AudioSourceComponent.h"

namespace Syn
{
    AudioSourceComponent::AudioSourceComponent()
        : soundIndex(UINT32_MAX)
        , play(false)
        , loop(false)
        , isSpatialized(true)
        , volume(1.0f)
        , pitch(1.0f)
        , minDistance(1.0f)
        , maxDistance(100.0f)
    {}
}