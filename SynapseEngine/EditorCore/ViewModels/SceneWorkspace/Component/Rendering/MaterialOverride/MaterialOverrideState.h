#pragma once
#include <cstdint>
#include <string>
#include <vector>
#include "EditorCore/Types/EntityHandle.h"

namespace Syn {
    struct MaterialOverrideState {
        bool hasComponent = false;

        uint32_t expectedSlotCount = 0;
        std::vector<uint32_t> overrides;
        EntityID sharedMaterialEntity = NULL_ENTITY;

        std::vector<std::pair<uint32_t, std::string>> availableMaterials;
        std::vector<std::pair<EntityID, std::string>> compatibleSharedEntities;
    };
}