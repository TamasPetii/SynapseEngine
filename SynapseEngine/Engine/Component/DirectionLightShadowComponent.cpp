#include "DirectionLightShadowComponent.h"

namespace Syn
{
    DirectionLightShadowComponent::DirectionLightShadowComponent() :
        shadowFarPlane(200.0f),
        cascadeSplits(glm::vec4(0.0f))
    {
        cascadeViews.fill(glm::mat4(1.0f));
        cascadeProjs.fill(glm::mat4(1.0f));
        cascadeViewProjs.fill(glm::mat4(1.0f));
		cascadeViewProjsVulkan.fill(glm::mat4(1.0f));
        cascadeAtlasRects.fill(glm::vec4(0.0f));
    }

    DirectionLightShadowGPU::DirectionLightShadowGPU(const DirectionLightShadowComponent& component) :
        cascadeSplits(component.cascadeSplits)
    {
        for (int i = 0; i < 4; ++i)
        {
            cascadeViewProjsVulkan[i] = component.cascadeViewProjsVulkan[i];
            cascadeAtlasRects[i] = component.cascadeAtlasRects[i];
        }
    }

    DirectionLightShadowColliderGPU::DirectionLightShadowColliderGPU(const DirectionLightShadowComponent& component, uint32_t entityIndex) :
        entityIndex(entityIndex)
    {
        for (int i = 0; i < 4; ++i)
        {
            for (int p = 0; p < 6; ++p)
            {
                cascades[i].planes[p] = component.cascadeFrustums[i].planes[p];
            }
        }
    }
}