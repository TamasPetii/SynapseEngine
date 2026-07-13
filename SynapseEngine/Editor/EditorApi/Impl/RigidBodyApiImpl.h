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