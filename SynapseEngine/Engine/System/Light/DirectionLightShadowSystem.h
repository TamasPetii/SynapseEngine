#pragma once
#include "Engine/SynApi.h"
#include "Engine/System/ComponentSystem.h"
#include "Engine/Component/DirectionLightShadowComponent.h"

namespace Syn
{
    class SYN_API DirectionLightShadowSystem : public ComponentSystem<DirectionLightShadowComponent>
    {
    public:
        std::string GetName() const override { return "DirectionLightShadowSystem"; }
        std::vector<TypeID> GetReadDependencies() const override;
        std::vector<TypeID> GetWriteDependencies() const override;

    protected:
        std::string GetSparseBufferName() const override { return BufferNames::DirectionLightShadowSparseMap; }

        void UpdateComponents(Scene* scene, uint32_t frameIndex, float deltaTime, tf::Subflow& subflow) override;
        void UploadComponents(Scene* scene, uint32_t frameIndex, tf::Subflow& subflow, bool uploadDynamic, bool uploadStatic) override;
    };
}