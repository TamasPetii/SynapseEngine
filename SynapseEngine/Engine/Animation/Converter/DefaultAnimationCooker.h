#pragma once
#include "IAnimationCooker.h"

namespace Syn
{
    class SYN_API DefaultAnimationCooker : public IAnimationCooker
    {
    public:
        DefaultAnimationCooker() = default;
        ~DefaultAnimationCooker() override = default;

        CookedAnimation Cook(const RawAnimation& rawAnimation, float sampleRate = 60.0f) override;
    };
}