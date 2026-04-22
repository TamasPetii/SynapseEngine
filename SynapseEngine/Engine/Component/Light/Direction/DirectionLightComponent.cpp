#include "DirectionLightComponent.h"

namespace Syn
{
    DirectionLightComponent::DirectionLightComponent() :
        Light(),
        direction(glm::vec3(0.0f, -1.0f, 0.0f))
    {}

    DirectionLightComponentGPU::DirectionLightComponentGPU(const DirectionLightComponent& component) :
        direction(component.direction),
        strength(component.strength),
        color(component.color),
        flags(0)
    {
        if (component.useShadow) flags |= (1u << 0);
    }
}