#include "DirectionLightComponent.h"

namespace Syn
{
    DirectionLightComponent::DirectionLightComponent() :
        Light(),
        direction(glm::vec3(0.0f, -1.0f, 0.0f)),
        shadowFarPlane(200.0f),
        cascadeSplits(glm::vec4(0.0f))
    {
        cascadeViewProjs.fill(glm::mat4(1.0f));
        cascadeAtlasRects.fill(glm::vec4(0.0f));
    }

    DirectionLightGPU::DirectionLightGPU(const DirectionLightComponent& component) :
        direction(component.direction),
        strength(component.strength),
        color(component.color),
        flags(0),
        cascadeSplits(component.cascadeSplits)
    {
        if (component.useShadow) flags |= (1u << 0);

        for (int i = 0; i < 4; ++i)
        {
            cascadeViewProjs[i] = component.cascadeViewProjs[i];
            cascadeAtlasRects[i] = component.cascadeAtlasRects[i];
        }
    }
}