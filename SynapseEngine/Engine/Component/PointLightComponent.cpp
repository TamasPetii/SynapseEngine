#include "PointLightComponent.h"

namespace Syn
{
    PointLightComponent::PointLightComponent() :
        Light(),
        position(0.0f),
        radius(10.0f),
        weakenDistance(0.0f)
    {}

    PointLightGPU::PointLightGPU(const PointLightComponent& component) :
        position(component.position),
        radius(component.radius),
        color(component.color),
        strength(component.strength),
        weakenDistance(component.weakenDistance),
        flags(0),
        padding(0),
        padding2(0)
    {
        if (component.useShadow) flags |= (1u << 0);
    }

    PointLightColliderGPU::PointLightColliderGPU(const PointLightComponent& component, uint32_t entityId) :
        center(component.position),
        radius(component.radius),
        entityIndex(entityId),
        padding0(0), padding1(0), padding2(0)
    {}
}