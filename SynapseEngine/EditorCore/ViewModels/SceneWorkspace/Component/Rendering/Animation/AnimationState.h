#pragma once
#include <cstdint>
#include <string>
#include <vector>

namespace Syn {
    struct AnimationState {
        bool hasComponent = false;
        float speed;
        uint32_t animationIndex;

        std::vector<std::pair<uint32_t, std::string>> availableAnimations;
    };
}