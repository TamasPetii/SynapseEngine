#pragma once
#include "Engine/SynApi.h"
#include "Engine/Animation/Data/Raw/RawAnimation.h"
#include "Engine/Animation/Data/Cooked/CookedAnimation.h"

namespace Syn
{
    class SYN_API IAnimationCooker
    {
    public:
        virtual ~IAnimationCooker() = default;
        virtual CookedAnimation Cook(const RawAnimation& rawAnimation, float sampleRate = 60.0f) = 0;
    };
}