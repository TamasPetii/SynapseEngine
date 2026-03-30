#include "PointLightShadowComponent.h"

namespace Syn
{
    PointLightShadowComponent::PointLightShadowComponent() :
        nearPlane(0.1f),
        farPlane(10.0f)
    {
        viewProjs.fill(glm::mat4(1.0f));
        atlasRects.fill(glm::vec4(0.0f));
    }

    PointLightShadowGPU::PointLightShadowGPU(const PointLightShadowComponent& component) :
        planes(component.nearPlane, component.farPlane, 0.0f, 0.0f)
    {
        for (int i = 0; i < 6; ++i)
        {
            viewProjs[i] = component.viewProjs[i];
            atlasRects[i] = component.atlasRects[i];
        }
    }
}