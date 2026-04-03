#pragma once
#include "Engine/SynApi.h"
#include "BaseComponent/Component.h"
#include <glm/glm.hpp>

namespace Syn
{
    struct SYN_API SpotLightShadowComponent : public Component
    {
        SpotLightShadowComponent();

        float nearPlane;
        float farPlane;
        glm::mat4 viewProj;
        glm::vec4 atlasRect;
    };

    struct SYN_API SpotLightShadowComponentGPU
    {
        SpotLightShadowComponentGPU(const SpotLightShadowComponent& component);

        glm::vec4 planes;
        glm::mat4 viewProj;
        glm::vec4 atlasRect;
    };
}