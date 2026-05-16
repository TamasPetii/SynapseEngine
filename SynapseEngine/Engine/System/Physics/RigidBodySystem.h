#pragma once
#include "Engine/System/ComponentSystem.h"
#include "Engine/Component/Physics/RigidBodyComponent.h"

namespace Syn
{
    class SYN_API RigidBodySystem : public ComponentSystem<RigidBodyComponent>
    {
    public:
        std::string GetName() const override { return "RigidBodySystem"; }
        std::string GetGroup() const override { return SystemGroupNames::PhysicsSystems; }

        std::vector<TypeID> GetReadDependencies() const override;
        std::vector<TypeID> GetWriteDependencies() const override;
    protected:
        void UpdateComponents(Scene* scene, uint32_t frameIndex, float deltaTime, tf::Subflow& subflow) override;
        void UploadComponents(Scene* scene, uint32_t frameIndex, tf::Subflow& subflow, bool uploadDynamic, bool uploadStatic) override {}
    };
}