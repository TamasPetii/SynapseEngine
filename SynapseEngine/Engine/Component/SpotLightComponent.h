#pragma once
#include "Engine/SynApi.h"
#include "BaseComponent/Component.h"
#include "BaseComponent/Light.h"
#include "Engine/Collision/Colliders/SphereCollider.h"
#include "Engine/Collision/Colliders/AabbCollider.h"
#include <glm/glm.hpp>

namespace Syn
{
    struct SYN_API SpotLightComponent : public Component, public Light
    {
        SpotLightComponent();

        glm::vec3 position;
        glm::vec3 direction;

        float range;
        float weakenDistance;
        float innerAngle;
        float outerAngle;
    private:
		AabbCollider aabbCollider;
        SphereCollider sphereCollider;
        friend struct SpotLightColliderGPU;
    };

    struct SYN_API SpotLightGPU
    {
        SpotLightGPU(const SpotLightComponent& component);

        glm::vec3 position;
        float range; 
        glm::vec3 direction;
		float weakenDistance;
        glm::vec3 color;
		float strength;

		float innerAngle;
		float outerAngle;
        float innerCosAngle;
		float outerCosAngle;

        uint32_t flags;
		uint32_t padding0;
        uint32_t padding1;
        uint32_t padding2;
    };

    struct SYN_API SpotLightColliderGPU
    {
        SpotLightColliderGPU(const SpotLightComponent& component, uint32_t entityIndex);

        glm::vec3 center;
        float radius;
        glm::vec3 aabbMin;
        uint32_t entityIndex;
        glm::vec3 aabbMax;
        uint32_t padding;
    };
}