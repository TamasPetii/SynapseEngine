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
#include "EditorCore/Types/EntityHandle.h"
#include "Engine/Physics/PhysicsTypes.h"
#include "IApi.h"
#include <cstdint>

namespace Syn {
    class IRigidBodyApi : public IApi {
    public:
        virtual ~IRigidBodyApi() = default;

        virtual bool HasRigidBody(EntityID entity) const = 0;

        virtual PhysicsMotionType GetRigidBodyMotionType(EntityID entity) const = 0;
        virtual float GetRigidBodyMass(EntityID entity) const = 0;
        virtual float GetRigidBodyFriction(EntityID entity) const = 0;
        virtual float GetRigidBodyRestitution(EntityID entity) const = 0;
        virtual uint32_t GetRigidBodyLayer(EntityID entity) const = 0;

        virtual void SetRigidBodyMotionType(EntityID entity, PhysicsMotionType motionType) = 0;
        virtual void SetRigidBodyMass(EntityID entity, float mass) = 0;
        virtual void SetRigidBodyFriction(EntityID entity, float friction) = 0;
        virtual void SetRigidBodyRestitution(EntityID entity, float restitution) = 0;
        virtual void SetRigidBodyLayer(EntityID entity, uint32_t layer) = 0;
    };
}