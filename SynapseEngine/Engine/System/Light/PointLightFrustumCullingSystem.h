#pragma once
#include "Engine/SynApi.h"
#include "Engine/System/ISystem.h"
#include <vector>

namespace Syn
{
    class SYN_API PointLightFrustumCullingSystem : public ISystem
    {
    public:
        std::string GetName() const override { return "PointLightFrustumCullingSystem"; }
        std::vector<TypeID> GetReadDependencies() const override;
        std::vector<TypeID> GetWriteDependencies() const override;

        void OnUpdate(Scene* scene, uint32_t frameIndex, float deltaTime, tf::Subflow& subflow) override;
        void OnUploadToGpu(Scene* scene, uint32_t frameIndex, tf::Subflow& subflow) override;
        void OnFinish(Scene* scene, tf::Subflow& subflow) override {}
    };
}