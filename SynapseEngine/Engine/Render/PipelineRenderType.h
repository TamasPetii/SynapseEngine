#pragma once
#include "Engine/SynApi.h"
#include <cstdint>

namespace Syn
{
    enum PipelineRenderType : uint32_t {
        Traditional = 0,
        Meshlet = 1,
        PipelineRenderTypeCount = 2
    };
}