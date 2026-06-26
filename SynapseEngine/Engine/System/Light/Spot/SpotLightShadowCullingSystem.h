#pragma once
#include "Engine/System/ISystem.h"
#include <vector>

namespace Syn
{
    struct SpotShadowSortData {
        uint32_t drawCallKey; // [Bit 31: isMeshlet] [Bit 0-30: indirectIdx]
        SpotShadowInstancePayload gpuPayload;

        bool operator<(const SpotShadowSortData& other) const {
            return drawCallKey < other.drawCallKey;
        }
    };

    class SYN_API SpotLightShadowCullingSystem : public ISystem
    {
    public:
        std::string GetName() const override { return "SpotLightShadowCullingSystem"; }
        std::string GetGroup() const override { return SystemGroupNames::SpotLightSystems; }

        std::vector<TypeID> GetReadDependencies() const override;

        void OnUpdate(Scene* scene, uint32_t frameIndex, float deltaTime, tf::Subflow& subflow) override;
        void OnUploadToGpu(Scene* scene, uint32_t frameIndex, tf::Subflow& subflow) override;
        void OnFinish(Scene* scene, tf::Subflow& subflow) override {}
    private:
        std::vector<SpotShadowSortData> _sortBuffer;
    };
}