#pragma once
#include "ComponentSystem.h"
#include "Engine/Component/AnimationComponent.h"

namespace Syn
{
    class SYN_API AnimationSystem : public ComponentSystem<AnimationComponent>
    {
    public:
        std::string GetName() const override { return "AnimationSystem"; }
        std::vector<TypeID> GetWriteDependencies() const override;
    protected:
        std::string GetSparseBufferName() const override { return BufferNames::AnimationSparseMap; }
    protected:
        void UpdateComponents(Scene* scene, uint32_t frameIndex, float deltaTime, tf::Subflow& subflow) override;
        void UploadComponents(Scene* scene, uint32_t frameIndex, tf::Subflow& subflow, bool uploadDynamic, bool uploadStatic) override;
    };
}