#pragma once
#include <variant>
#include <cstdint>

namespace Syn
{
    struct SetModelCastShadowIntent {
        bool castShadow;
    };

    struct SetModelReceiveShadowIntent {
        bool receiveShadow;
    };

    struct SetModelIndexIntent {
        uint32_t modelIndex;
    };

    using ModelComponentIntent = std::variant<
        SetModelCastShadowIntent,
        SetModelReceiveShadowIntent,
        SetModelIndexIntent
    >;
}