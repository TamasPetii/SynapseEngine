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
#include "EditorCore/Api/IRigidBodyApi.h"
#include "Engine/Scene/SceneManager.h"

namespace Syn {
    class RigidBodyApiImpl : public IRigidBodyApi {
    public:
        RigidBodyApiImpl(SceneManager* sm) : _sceneManager(sm) {}

        bool HasRigidBody(EntityID entity) const override;

        PhysicsMotionType GetRigidBodyMotionType(EntityID entity) const override;
        float GetRigidBodyMass(EntityID entity) const override;
        float GetRigidBodyFriction(EntityID entity) const override;
        float GetRigidBodyRestitution(EntityID entity) const override;
        uint32_t GetRigidBodyLayer(EntityID entity) const override;

        void SetRigidBodyMotionType(EntityID entity, PhysicsMotionType motionType) override;
        void SetRigidBodyMass(EntityID entity, float mass) override;
        void SetRigidBodyFriction(EntityID entity, float friction) override;
        void SetRigidBodyRestitution(EntityID entity, float restitution) override;
        void SetRigidBodyLayer(EntityID entity, uint32_t layer) override;

    private:
        SceneManager* _sceneManager;
    };
}