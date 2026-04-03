#pragma once
#include "Engine/SynApi.h"
#include "Engine/Image/Data/Cooked/CookedImage.h"
#include "Engine/Image/Data/Raw/RawImage.h"

namespace Syn
{
    class SYN_API IImageCooker
    {
    public:
        virtual ~IImageCooker() = default;
        virtual CookedImage Cook(const RawImage& rawImage) = 0;
    };
}