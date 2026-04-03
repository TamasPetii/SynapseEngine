#pragma once
#include "Engine/SynApi.h"
#include "IImageCooker.h"

namespace Syn
{
    class SYN_API DefaultImageCooker : public IImageCooker
    {
    public:
        DefaultImageCooker() = default;
        ~DefaultImageCooker() override = default;

        CookedImage Cook(const RawImage& rawImage) override;
    };
}