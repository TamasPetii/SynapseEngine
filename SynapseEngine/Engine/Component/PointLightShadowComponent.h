#pragma once
#include "Engine/SynApi.h"
#include "BaseComponent/Component.h"
#include <glm/glm.hpp>
#include <array>

namespace Syn
{
    struct SYN_API PointLightShadowComponent : public Component
    {
        PointLightShadowComponent();

        float nearPlane;
        float farPlane;

        std::array<glm::mat4, 6> viewProjs;
        std::array<glm::vec4, 6> atlasRects;
    };

    struct SYN_API PointLightShadowComponentGPU
    {
        PointLightShadowComponentGPU(const PointLightShadowComponent& component);

        glm::vec4 planes;
        glm::mat4 viewProjs[6];
        glm::vec4 atlasRects[6];
    };
}