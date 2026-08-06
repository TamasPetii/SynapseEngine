#pragma once
#include "Engine/Animation/Data/Cpu/CpuAnimationData.h"

namespace Syn {
    struct AnimationSequencerState {
        uint32_t activeAnimationId = 0xFFFFFFFF;
        const CpuAnimationData* currentAnimData = nullptr;

        int32_t currentFrame = 0;
        int32_t selectedTrackIndex = -1;
    };
}