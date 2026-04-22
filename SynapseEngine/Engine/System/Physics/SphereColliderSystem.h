#pragma once
#include "Engine/System/ComponentSystem.h"
#include "Engine/Component/Physics/SphereColliderComponent.h"

namespace Syn
{
    class SYN_API SphereColliderSystem : public ComponentSystem<SphereColliderComponent>
    {
    public:
        std::string GetName() const override { return "SphereColliderSystem"; }
        std::vector<TypeID> GetReadDependencies() const override;
        std::vector<TypeID> GetWriteDependencies() const override;
    protected:
        std::string GetSparseBufferName() const override { return BufferNames::SphereColliderSparseMap; }
    protected:
        void UpdateComponents(Scene* scene, uint32_t frameIndex, float deltaTime, tf::Subflow& subflow) override;
        void UploadComponents(Scene* scene, uint32_t frameIndex, tf::Subflow& subflow, bool uploadDynamic, bool uploadStatic) override;
    };
}