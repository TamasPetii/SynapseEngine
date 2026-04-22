#include "SpotLightShadowComponent.h"

namespace Syn
{
    SpotLightShadowComponent::SpotLightShadowComponent() :
        nearPlane(0.1f),
        farPlane(15.0f),
        viewProj(1.0f),
        atlasRect(0.0f)
    {}

    SpotLightShadowComponentGPU::SpotLightShadowComponentGPU(const SpotLightShadowComponent& component) :
        planes(component.nearPlane, component.farPlane, 0.0f, 0.0f),
        viewProj(component.viewProj),
        atlasRect(component.atlasRect)
    {}
}