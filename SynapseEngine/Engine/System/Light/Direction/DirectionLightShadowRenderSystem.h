#pragma once
#include "Engine/System/ISystem.h"
#include <vector>

namespace Syn
{
    class SYN_API DirectionLightShadowRenderSystem : public ISystem
    {
    public:
        std::string GetName() const override { return "DirectionLightShadowRenderSystem"; }
        std::string GetGroup() const override { return SystemGroupNames::RenderingSystems; }

        std::vector<TypeID> GetReadDependencies() const override;
        std::vector<TypeID> GetWriteDependencies() const override;

        void OnUpdate(Scene* scene, uint32_t frameIndex, float deltaTime, tf::Subflow& subflow) override;
        void OnUploadToGpu(Scene* scene, uint32_t frameIndex, tf::Subflow& subflow) override;
        void OnFinish(Scene* scene, tf::Subflow& subflow) override;
    private:
        void RebuildShadowBuffers(Scene* scene);
    private:
        uint32_t _lastMainAllocatedInstances = 0;
        bool _needsRebuild = true;
    };
}