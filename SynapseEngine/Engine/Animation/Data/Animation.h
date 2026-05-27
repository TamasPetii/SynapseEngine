#pragma once
#include "Engine/SynApi.h"
#include "Engine/Animation/Data/Cooked/CookedAnimation.h"
#include "Engine/Animation/Data/Gpu/GpuBatchedAnimation.h"
#include "Engine/Animation/Data/Gpu/GpuAnimationBuffers.h"
#include "Engine/Animation/Data/Cpu/CpuAnimationData.h"


namespace Syn
{
    struct SYN_API Animation
    {
        CpuAnimationData cpuData;
        GpuAnimationBuffers hardwareBuffers;

        std::unique_ptr<CookedAnimation> transientCpuData;
        std::unique_ptr<GpuBatchedAnimation> transientGpuData;
    };
}