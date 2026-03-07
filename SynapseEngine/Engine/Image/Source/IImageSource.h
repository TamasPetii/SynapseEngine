#pragma once
#include "Engine/SynApi.h"
#include "Engine/Image/Data/Raw/RawImage.h"
#include <optional>

namespace Syn
{
    class SYN_API IImageSource
    {
    public:
        virtual ~IImageSource() = default;
        virtual std::optional<RawImage> Produce() = 0;
    };
}