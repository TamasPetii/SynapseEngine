#pragma once
#include "ComponentSystem.h"
#include "Engine/Component/ModelComponent.h"

namespace Syn
{
    class SYN_API ModelSystem : public ComponentSystem<ModelComponent>
    {
    public:
        std::string GetName() const override { return "ModelSystem"; }
        std::vector<TypeID> GetReadDependencies() const override;
        std::vector<TypeID> GetWriteDependencies() const override;
    protected:
        std::string GetSparseBufferName() const override { return BufferNames::ModelSparseMap; }
    protected:
        void UpdateComponents(Scene* scene, uint32_t frameIndex, float deltaTime, tf::Subflow& subflow) override;
        void UploadComponents(Scene* scene, uint32_t frameIndex, tf::Subflow& subflow, bool uploadDynamic) override;
    };
}