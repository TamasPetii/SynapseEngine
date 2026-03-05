#include "Engine/SynApi.h"
#include <cstdint>

namespace Syn {
    struct SYN_API FrameContext {
        uint32_t currentFrameIndex = 0;
        uint32_t framesInFlight = 2;
        float deltaTime = 0.0f;
    };
}