#pragma once
#include <glm/glm.hpp>
#include <variant>

namespace Syn
{
    struct SetSpotLightColorIntent
    {
        glm::vec3 color;
        bool isDragging;
    };
    
    struct SetSpotLightStrengthIntent
    {
        float strength;
        bool isDragging;
    };

    struct SetSpotLightUseShadowIntent
    {
        bool useShadow;
    };

    struct SetSpotLightRangeIntent
    {
        float range;
        bool isDragging;
    };

    struct SetSpotLightWeakenIntent
    {
        float distance;
        bool isDragging;
    };

    struct SetSpotLightInnerAngleIntent
    {
        float angle;
        bool isDragging;
    };

    struct SetSpotLightOuterAngleIntent
    {
        float angle;
        bool isDragging;
    };

    struct SetSpotLightShadowNearIntent
    {
        float nearPlane;
        bool isDragging;
    };

    struct SetSpotLightShadowFarIntent
    {
        float farPlane;
        bool isDragging;
    };

    using SpotLightIntent = std::variant<
        SetSpotLightColorIntent, SetSpotLightStrengthIntent, SetSpotLightUseShadowIntent,
        SetSpotLightRangeIntent, SetSpotLightWeakenIntent, SetSpotLightInnerAngleIntent,
        SetSpotLightOuterAngleIntent, SetSpotLightShadowNearIntent, SetSpotLightShadowFarIntent>;
}