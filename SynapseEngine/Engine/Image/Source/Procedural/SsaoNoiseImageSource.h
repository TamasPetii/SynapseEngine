#pragma once
#include "ProceduralImageSource.h"

namespace Syn
{
    class SYN_API SsaoNoiseImageSource : public ProceduralImageSource
    {
    public:
        SsaoNoiseImageSource();
        std::optional<RawImage> Produce() override;
    };
}