#pragma once
#include "Engine/Animation/Data/Cpu/CpuAnimationData.h"
#include <vector>

namespace Syn {
    struct SequencerTrackUIState {
        bool groupOpen = true;
        std::vector<int32_t> positionFrames;
        std::vector<int32_t> rotationFrames;
        std::vector<int32_t> scaleFrames;
    };

    struct AnimationSequencerState {
        uint32_t activeAnimationId = 0xFFFFFFFF;
        const CpuAnimationData* currentAnimData = nullptr;

        int32_t currentFrame = 0;
        int32_t selectedTrackIndex = -1;

        std::vector<BoneTrack> editableTracks;
        std::vector<SequencerTrackUIState> trackUIStates;
    };
}