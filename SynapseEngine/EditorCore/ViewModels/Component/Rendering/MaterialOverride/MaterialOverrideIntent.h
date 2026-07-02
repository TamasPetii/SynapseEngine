#pragma once
#include <variant>
#include <cstdint>
#include "EditorCore/Types/EntityHandle.h"

namespace Syn
{
    struct SetMaterialOverrideSlotIntent {
        uint32_t slotIndex;
        uint32_t materialId;
    };

    struct SetSharedMaterialEntityIntent {
        EntityID sharedEntity;
    };

    using MaterialOverrideIntent = std::variant<
        SetMaterialOverrideSlotIntent,
        SetSharedMaterialEntityIntent
    >;
}