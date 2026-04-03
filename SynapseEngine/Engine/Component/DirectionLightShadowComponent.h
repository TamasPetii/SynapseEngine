#pragma once
#include "Engine/SynApi.h"
#include "BaseComponent/Component.h"
#include "Engine/Collision/Colliders/FrustumCollider.h" // Aabb és Sphere helyett
#include <glm/glm.hpp>
#include <array>

namespace Syn
{
    struct SYN_API DirectionLightShadowComponent : public Component
    {
        DirectionLightShadowComponent();

        float shadowFarPlane;
        glm::vec4 cascadeSplits;

        std::array<glm::mat4, 4> cascadeViews;
        std::array<glm::mat4, 4> cascadeProjs;
        std::array<glm::mat4, 4> cascadeViewProjs;
        std::array<glm::mat4, 4> cascadeViewProjsVulkan;
        std::array<glm::vec4, 4> cascadeAtlasRects;
    private:
        std::array<FrustumCollider, 4> cascadeFrustums;

        friend struct DirectionLightShadowColliderGPU;
        friend class DirectionLightShadowSystem;
    };

    struct SYN_API DirectionLightShadowGPU
    {
        DirectionLightShadowGPU(const DirectionLightShadowComponent& component);

        glm::vec4 cascadeSplits;
        glm::mat4 cascadeViewProjsVulkan[4];
        glm::vec4 cascadeAtlasRects[4];
    };

    struct SYN_API DirectionLightShadowColliderGPU
    {
        DirectionLightShadowColliderGPU(const DirectionLightShadowComponent& component, uint32_t entityIndex);

        struct CascadeCollider {
            glm::vec4 planes[6];
        } cascades[4];

        uint32_t entityIndex;
        uint32_t padding[3];
    };
}