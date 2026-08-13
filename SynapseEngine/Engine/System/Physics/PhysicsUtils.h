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
#include "Engine/SynMacro.h"
#include "Engine/Component/Core/TransformComponent.h"
#include "Engine/Component/Physics//RigidBodyComponent.h"
#include "Engine/Physics/PhysicsTypes.h"
#include "Engine/Registry/Entity.h"
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

namespace Syn
{
    class SYN_API PhysicsUtils
    {
    public:
        SYN_INLINE static glm::vec3 ExtractScale(const glm::mat4& mat);

        template<typename F>
        SYN_INLINE static PhysicsBodyID TryCreateBody(EntityID entity, TransformComponent* tr, RigidBodyComponent& rb, F&& createShapeFunc);
    };

    SYN_INLINE glm::vec3 PhysicsUtils::ExtractScale(const glm::mat4& mat)
    {
        return glm::vec3(
            glm::length(glm::vec3(mat[0])),
            glm::length(glm::vec3(mat[1])),
            glm::length(glm::vec3(mat[2]))
        );
    }

    template<typename F>
    SYN_INLINE PhysicsBodyID PhysicsUtils::TryCreateBody(EntityID entity, TransformComponent* tr, RigidBodyComponent& rb, F&& createShapeFunc)
    {
        if (rb.bodyID != INVALID_BODY_ID)
            return rb.bodyID;
        if (!tr)
            return INVALID_BODY_ID;

        const glm::mat4& mat = tr->transform;

        glm::vec3 worldScale = ExtractScale(mat);
        glm::vec3 worldTranslation = glm::vec3(mat[3]);

        glm::mat3 rotMat(
            glm::vec3(mat[0]) / worldScale.x,
            glm::vec3(mat[1]) / worldScale.y,
            glm::vec3(mat[2]) / worldScale.z
        );

        glm::quat worldRotation = glm::quat_cast(rotMat);

        PhysicsBodySettings settings;
        settings.motionType = rb.motionType;
        settings.mass = rb.mass;
        settings.friction = rb.friction;
        settings.restitution = rb.restitution;
        settings.layer = rb.layer;

        return createShapeFunc(worldTranslation, worldRotation, worldScale, settings);
    }
}