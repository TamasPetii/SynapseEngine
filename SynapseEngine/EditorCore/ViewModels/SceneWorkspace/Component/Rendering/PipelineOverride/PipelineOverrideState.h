#pragma once
#include <cstdint>
#include <string>
#include <vector>
#include "EditorCore/Types/EntityHandle.h"

namespace Syn {
    struct PipelineOverrideState {
        bool hasComponent = false;

        uint32_t expectedSlotCount = 0;
        std::vector<uint32_t> overrides;
        EntityID sharedPipelineEntity = NULL_ENTITY;

        std::vector<std::pair<uint32_t, std::string>> availablePipelines;
        std::vector<std::pair<EntityID, std::string>> compatibleSharedEntities;
    };
}