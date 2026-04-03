#pragma once
#include "Engine/SynApi.h"
#include "Engine/System/ComponentSystem.h"
#include "Engine/Component/SpotLightShadowComponent.h"

namespace Syn
{
    class SYN_API SpotLightShadowSystem : public ComponentSystem<SpotLightShadowComponent>
    {
    public:
        std::string GetName() const override { return "SpotLightShadowSystem"; }
        std::vector<TypeID> GetReadDependencies() const override;
        std::vector<TypeID> GetWriteDependencies() const override;

    protected:
        std::string GetSparseBufferName() const override { return BufferNames::SpotLightShadowSparseMap; }

        void UpdateComponents(Scene* scene, uint32_t frameIndex, float deltaTime, tf::Subflow& subflow) override;
        void UploadComponents(Scene* scene, uint32_t frameIndex, tf::Subflow& subflow, bool uploadDynamic, bool uploadStatic) override;
    };
}