#pragma once
#include "Engine/SynApi.h"

namespace Syn
{
    struct SYN_API SamplerNames
    {
        static constexpr const char* Linear = "Linear";
        static constexpr const char* Nearest = "Nearest";
        static constexpr const char* LinearAniso = "LinearAniso";
        static constexpr const char* NearestAniso = "NearestAniso";
        static constexpr const char* MaxReduction = "MaxReduction";
        static constexpr const char* BloomSampler = "BloomSampler";
        static constexpr const char* ShadowSampler = "ShadowSampler";
    };
}