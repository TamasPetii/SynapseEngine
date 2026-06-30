#pragma once
#include "EditorCore/Types/EntityHandle.h"
#include "Engine/Physics/PhysicsTypes.h"
#include <cstdint>

namespace Syn {
    class IRigidBodyApi {
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