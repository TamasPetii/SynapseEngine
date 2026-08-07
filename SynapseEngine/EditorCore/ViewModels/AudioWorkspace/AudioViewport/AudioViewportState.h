#pragma once
#include <cstdint>
#include "Engine/Audio/Data/Cpu/CpuAudioData.h"

namespace Syn {
    struct AudioViewportState {
        uint32_t activeAudioId = 0xFFFFFFFF;
        const CpuAudioData* currentAudioData = nullptr;

        bool isPlaying = false;
        float currentTime = 0.0f;
        float duration = 0.0f;
    };
}