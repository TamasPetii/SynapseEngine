#pragma once
#include "ComponentSystem.h"
#include "Engine/Component/TransformComponent.h"

namespace Syn
{
    class SYN_API TransformSystem : public ComponentSystem<TransformComponent>
    {
    public:
        std::string GetName() const override { return "TransformSystem"; }
        std::vector<TypeID> GetWriteDependencies() const override;
    protected:
        std::string GetSparseBufferName() const override { return BufferNames::TransformSparseMap; }
    protected:
        void UpdateComponents(Scene* scene, uint32_t frameIndex, float deltaTime, tf::Subflow& subflow) override;
        void UploadComponents(Scene* scene, uint32_t frameIndex, tf::Subflow& subflow, bool uploadDynamic, bool uploadStatic) override;
    };
}