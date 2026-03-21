#pragma once
#include "Engine/SynApi.h"
#include "Engine/Mesh/Data/Cooked/CookedModel.h"
#include "Engine/Animation/Data/Cooked/CookedAnimation.h"

namespace Syn
{
    class SYN_API IAnimationProcessor
    {
    public:
        virtual ~IAnimationProcessor() = default;
        virtual void Process(CookedAnimation& inOutAnimation, const CookedModel& baseModel) = 0;
    };
}