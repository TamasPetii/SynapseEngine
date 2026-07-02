#pragma once
#include <variant>
#include <cstdint>

namespace Syn
{
    struct SetAnimationSpeedIntent
    {
        float speed;
        bool isDragging;
    };

    struct SetAnimationIndexIntent
    {
        uint32_t animationIndex;
    };

    using AnimationIntent = std::variant<SetAnimationSpeedIntent, SetAnimationIndexIntent>;
}