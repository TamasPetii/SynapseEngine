#pragma once
#include "Engine/SynApi.h"
#include "Engine/Animation/Data/Cooked/CookedAnimation.h"
#include "Engine/Animation/Data/Gpu/GpuBatchedAnimation.h"
#include "Engine/Animation/Data/Gpu/GpuAnimationBuffers.h"

namespace Syn
{
    struct SYN_API Animation
    {
        CookedAnimation cpuData;
        GpuBatchedAnimation gpuData;
        GpuAnimationBuffers hardwareBuffers;
    };
}