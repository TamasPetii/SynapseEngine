#pragma once
#include <cstdint>
#include <string>
#include <vector>

namespace Syn {
    struct ModelComponentState {
        bool hasComponent = false;
        bool castShadow;
        bool receiveShadow;
        uint32_t modelIndex;

        std::vector<std::pair<uint32_t, std::string>> availableModels;
    };
}