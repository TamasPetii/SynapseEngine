#pragma once
#include <glm/glm.hpp>
#include <variant>

namespace Syn {
    struct SetLightColorIntent { 
        glm::vec3 color;
        bool isDragging;
    };

    struct SetLightStrengthIntent { 
        float strength;
        bool isDragging;
    };

    struct SetLightUseShadowIntent { 
        bool useShadow;
    };

    struct SetShadowFarPlaneIntent { 
        float farPlane;
        bool isDragging;
    };

    struct SetCascadeSplitsIntent { 
        glm::vec4 splits;
        bool isDragging;
    };

    using DirectionLightIntent = std::variant<
        SetLightColorIntent,
        SetLightStrengthIntent,
        SetLightUseShadowIntent,
        SetShadowFarPlaneIntent,
        SetCascadeSplitsIntent
    >;
}