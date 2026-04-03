#pragma once
#include "Engine/SynApi.h"
#include "BaseComponent/Component.h"
#include "BaseComponent/Light.h"
#include <glm/glm.hpp>
#include <array>

namespace Syn
{
    struct SYN_API PointLightComponent : public Component, public Light
    {
        PointLightComponent();

        glm::vec3 position;
        float radius;
        float weakenDistance;
    };

    struct SYN_API PointLightComponentGPU
    {
        PointLightComponentGPU(const PointLightComponent& component);

        glm::vec3 position;
        float radius;
        glm::vec3 color;
		float strength;
        float weakenDistance;
        uint32_t flags;
        uint32_t padding;
        uint32_t padding2;
    };

    struct SYN_API PointLightColliderGPU
    {
        PointLightColliderGPU(const PointLightComponent& component, uint32_t entityId);

        glm::vec3 center;
        float radius;
        uint32_t entityIndex;

        uint32_t padding0;
        uint32_t padding1;
        uint32_t padding2;
    };
}