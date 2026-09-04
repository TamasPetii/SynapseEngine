// Copyright (C) 2026 Tamás Péter
// This file is part of SynapseEngine.
//
// SynapseEngine is free software: you can redistribute it and/or modify
// it under the terms of the GNU Affero General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// SynapseEngine is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License
// along with SynapseEngine. If not, see <https://www.gnu.org/licenses/>.

#pragma once
#include "Engine/SynApi.h"
#include "Engine/Component/Core/Component.h"
#include "Engine/Collision/Colliders/FrustumCollider.h"
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

        std::array<glm::vec3, 4> cascadeAnchors;
        std::array<float, 4> cascadeRadius;
        std::array<bool, 4> isStaticDirty;
    private:
        std::array<glm::vec3, 4> cascadeAabbMin;
        std::array<glm::vec3, 4> cascadeAabbMax;
        std::array<FrustumCollider, 4> cascadeFrustums;

        friend struct DirectionLightShadowColliderGPU;
        friend class DirectionLightShadowSystem;
        friend class DirectionLightShadowCullingSystem;
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
            glm::vec4 aabbMin;
            glm::vec4 aabbMax;
        } cascades[4];

        uint32_t entityIndex;
        uint32_t padding[3];
    };
}