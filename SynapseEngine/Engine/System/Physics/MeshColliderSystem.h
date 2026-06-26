#pragma once
#include "Engine/System/ComponentSystem.h"
#include "Engine/Component/Physics/MeshColliderComponent.h"

namespace Syn
{
    class SYN_API MeshColliderSystem : public ComponentSystem<MeshColliderComponent>
    {
    public:
        std::string GetName() const override { return "MeshColliderSystem"; }
        std::string GetGroup() const override { return SystemGroupNames::PhysicsSystems; }

        std::vector<TypeID> GetReadDependencies() const override;
        std::vector<TypeID> GetWriteDependencies() const override;
    protected:
        std::string GetSparseBufferName() const override { return BufferNames::MeshColliderSparseMap; }
    protected:
        void UpdateComponents(Scene* scene, uint32_t frameIndex, float deltaTime, tf::Subflow& subflow) override;
        void UploadComponents(Scene* scene, uint32_t frameIndex, tf::Subflow& subflow, bool uploadDynamic, bool uploadStatic) override;
    };
}