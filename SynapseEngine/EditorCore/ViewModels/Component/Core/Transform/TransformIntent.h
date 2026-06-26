#pragma once
#include <glm/glm.hpp>
#include <variant>

namespace Syn {
    struct SetPositionIntent {
        glm::vec3 newPosition;
        bool isDragging;
    };

    struct SetRotationIntent {
        glm::vec3 newRotation;
        bool isDragging;
    };

    struct SetScaleIntent {
        glm::vec3 newScale;
        bool isDragging;
    };

    using TransformIntent = std::variant<
        SetPositionIntent,
        SetRotationIntent,
        SetScaleIntent
    >;
}