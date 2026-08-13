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

#include "RigidBodyApiImpl.h"
#include "../EditorApiUtils.h"
#include "Engine/Component/Physics/RigidBodyComponent.h"

namespace Syn {

    bool RigidBodyApiImpl::HasRigidBody(EntityID entity) const {
        return EditorApiUtils::HasComponent<RigidBodyComponent>(_sceneManager, entity);
    }

    PhysicsMotionType RigidBodyApiImpl::GetRigidBodyMotionType(EntityID entity) const {
        return EditorApiUtils::ReadComponent<RigidBodyComponent>(_sceneManager, entity, [](const auto& c) { return c.motionType; }, PhysicsMotionType::Dynamic);
    }

    float RigidBodyApiImpl::GetRigidBodyMass(EntityID entity) const {
        return EditorApiUtils::ReadComponent<RigidBodyComponent>(_sceneManager, entity, [](const auto& c) { return c.mass; }, 1.0f);
    }

    float RigidBodyApiImpl::GetRigidBodyFriction(EntityID entity) const {
        return EditorApiUtils::ReadComponent<RigidBodyComponent>(_sceneManager, entity, [](const auto& c) { return c.friction; }, 0.2f);
    }

    float RigidBodyApiImpl::GetRigidBodyRestitution(EntityID entity) const {
        return EditorApiUtils::ReadComponent<RigidBodyComponent>(_sceneManager, entity, [](const auto& c) { return c.restitution; }, 0.0f);
    }

    uint32_t RigidBodyApiImpl::GetRigidBodyLayer(EntityID entity) const {
        return EditorApiUtils::ReadComponent<RigidBodyComponent>(_sceneManager, entity, [](const auto& c) { return c.layer; }, 1u);
    }

    void RigidBodyApiImpl::SetRigidBodyMotionType(EntityID entity, PhysicsMotionType motionType) {
        EditorApiUtils::ModifyComponent<RigidBodyComponent>(_sceneManager, entity, [&](auto& c, auto pool) { c.motionType = motionType; });
    }

    void RigidBodyApiImpl::SetRigidBodyMass(EntityID entity, float mass) {
        EditorApiUtils::ModifyComponent<RigidBodyComponent>(_sceneManager, entity, [&](auto& c, auto pool) { c.mass = mass; });
    }

    void RigidBodyApiImpl::SetRigidBodyFriction(EntityID entity, float friction) {
        EditorApiUtils::ModifyComponent<RigidBodyComponent>(_sceneManager, entity, [&](auto& c, auto pool) { c.friction = friction; });
    }

    void RigidBodyApiImpl::SetRigidBodyRestitution(EntityID entity, float restitution) {
        EditorApiUtils::ModifyComponent<RigidBodyComponent>(_sceneManager, entity, [&](auto& c, auto pool) { c.restitution = restitution; });
    }

    void RigidBodyApiImpl::SetRigidBodyLayer(EntityID entity, uint32_t layer) {
        EditorApiUtils::ModifyComponent<RigidBodyComponent>(_sceneManager, entity, [&](auto& c, auto pool) { c.layer = layer; });
    }
}