#pragma once
#include "Engine/SynApi.h"
#include "ProceduralImageSource.h"

namespace Syn
{
    class SYN_API DefaultImageSource : public ProceduralImageSource
    {
    public:
        DefaultImageSource();
        ~DefaultImageSource() override = default;

        std::optional<RawImage> Produce() override;
    };
}