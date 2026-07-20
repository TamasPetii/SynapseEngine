#pragma once
#include <variant>
#include <cstdint>
#include "EditorCore/Types/EntityHandle.h"

namespace Syn
{
    struct SetPipelineOverrideSlotIntent {
        uint32_t slotIndex;
        uint32_t pipelineType;
    };

    struct SetSharedPipelineEntityIntent {
        EntityID sharedEntity;
    };

    using PipelineOverrideIntent = std::variant<
        SetPipelineOverrideSlotIntent,
        SetSharedPipelineEntityIntent
    >;
}