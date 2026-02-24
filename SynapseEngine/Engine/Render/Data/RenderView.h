#pragma once
#include "Engine/SynApi.h"
#include <glm/glm.hpp>
#include <cstdint>

namespace Syn
{
    struct SYN_API RenderView
    {
        glm::mat4 viewMatrix;
        glm::mat4 projectionMatrix;
        glm::mat4 viewProjectionMatrix;
        glm::vec3 position;

        uint32_t indirectCommandOffset = 0;
        uint32_t indirectCommandCount = 0;
        uint32_t instanceDataOffset = 0;
    };

    struct SYN_API CameraRenderView : public RenderView
    {

    };

    struct SYN_API PointLightShadowView : public RenderView 
    {

    };

    struct SYN_API SpotLightShadowView : public RenderView 
    {

    };
}