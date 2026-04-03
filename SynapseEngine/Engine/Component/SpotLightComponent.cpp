#include "SpotLightComponent.h"
#include <cmath>
#include <glm/gtc/matrix_transform.hpp>

namespace Syn
{
    SpotLightComponent::SpotLightComponent() :
        Light(),
        position(0.0f),
        direction(0.0f, 0.0f, -1.0f),
        range(15.0f),
        weakenDistance(0.0f),
        innerAngle(30.0f),
        outerAngle(45.0f)
    {}

    SpotLightComponentGPU::SpotLightComponentGPU(const SpotLightComponent& component) :
        position(component.position),
        range(component.range),
        direction(component.direction),
        weakenDistance(component.weakenDistance),
        color(component.color),
        strength(component.strength),
        innerAngle(component.innerAngle),
        outerAngle(component.outerAngle),
        innerCosAngle(std::cos(glm::radians(component.innerAngle))),
        outerCosAngle(std::cos(glm::radians(component.outerAngle))),
        flags(0),
        padding0(0),
        padding1(0),
        padding2(0)
    {
        if (component.useShadow) flags |= (1u << 0);
    }

    SpotLightColliderGPU::SpotLightColliderGPU(const SpotLightComponent& component, uint32_t entityId) :
        center(component.sphereCollider.center),
        radius(component.sphereCollider.radius),
        aabbMin(component.aabbCollider.min),
        entityIndex(entityId),
        aabbMax(component.aabbCollider.max),
        padding(0)
    {}
}