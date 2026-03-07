#pragma once
#include "Engine/SynApi.h"
#include "Engine/Image/Data/Cooked/CookedImage.h"

namespace Syn
{
    class SYN_API IImageProcessor
    {
    public:
        virtual ~IImageProcessor() = default;
        virtual void Process(CookedImage& cookedImage) = 0;
    };
}