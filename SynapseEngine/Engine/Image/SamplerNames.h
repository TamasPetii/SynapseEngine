#pragma once
#include "Engine/SynApi.h"

namespace Syn
{
    struct SYN_API SamplerNames
    {
        static constexpr const char* LinearRepeat = "LinearRepeat";
        static constexpr const char* LinearClampEdge = "LinearClampEdge";
        static constexpr const char* NearestRepeat = "NearestRepeat";
        static constexpr const char* NearestClampEdge = "NearestClampEdge";
        static constexpr const char* LinearAniso = "LinearAniso";
        static constexpr const char* NearestAniso = "NearestAniso";
        static constexpr const char* MaxReduction = "MaxReduction";
        static constexpr const char* BloomSampler = "BloomSampler";
        static constexpr const char* ShadowSampler = "ShadowSampler";
        static constexpr const char* SkyboxSampler = "SkyboxSampler";
    };

    struct SYN_API SamplerWrapModeNames
    {
        static constexpr const char* Repeat = "Repeat";
        static constexpr const char* ClampEdge = "ClampEdge";
    };
}