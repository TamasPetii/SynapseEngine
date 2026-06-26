#pragma once
#include <glm/glm.hpp>
#include <variant>

namespace Syn
{
    struct SetPointLightColorIntent
    {
        glm::vec3 color;
        bool isDragging;
    };

    struct SetPointLightStrengthIntent
    {
        float strength;
        bool isDragging;
    };

    struct SetPointLightUseShadowIntent
    {
        bool useShadow;
    };

    struct SetPointLightRadiusIntent
    {
        float radius;
        bool isDragging;
    };

    struct SetPointLightWeakenIntent
    {
        float distance;
        bool isDragging;
    };

    struct SetPointLightShadowNearIntent
    {
        float nearPlane;
        bool isDragging;
    };
    
    struct SetPointLightShadowFarIntent
    {
        float farPlane;
        bool isDragging;
    };

    using PointLightIntent = std::variant<
        SetPointLightColorIntent,
        SetPointLightStrengthIntent,
        SetPointLightUseShadowIntent,
        SetPointLightRadiusIntent,
        SetPointLightWeakenIntent,
        SetPointLightShadowNearIntent,
        SetPointLightShadowFarIntent>;
}