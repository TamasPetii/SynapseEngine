#pragma once
#include "Engine/SynApi.h"
#include "Engine/Statistics/IRenderStatCollector.h"

#include "Engine/Utils/RenderBuffer.h"
#include <vector>
#include <map>
#include <string>

namespace Syn 
{
    class Scene;

    enum class SYN_API StatCategory {
        Scene,
        DirectionalShadow,
        PointShadow,
        SpotShadow
    };

    struct SYN_API CpuRenderStats {
        uint32_t totalModels = 0;
        uint32_t totalDrawDescriptors = 0;
        uint32_t traditionalDrawDescriptors = 0;
        uint32_t meshletDrawDescriptors = 0;
        uint32_t totalAllocatedInstances = 0;
        uint32_t totalMaxMeshlets = 0;

        uint64_t maxPossibleVertices = 0;
        uint64_t maxPossibleIndices = 0;
        uint64_t maxPossibleTriangles = 0;

        uint32_t totalLights = 0;
        uint32_t visibleLights = 0;
        uint32_t visibleShadowLights = 0;
        uint32_t appendedInstances = 0;
    };

    struct SYN_API GpuCullingReadback {
        uint32_t spotVisibleLights;
        uint32_t spotVisibleShadowLights;
        uint32_t pointVisibleLights;
        uint32_t pointVisibleShadowLights;
    };

    class SYN_API FrameStatisticsManager {
    public:
        FrameStatisticsManager(uint32_t framesInFlight);

        void ResolveFrame(VkCommandBuffer cmd, Scene* scene, uint32_t frameIndex, const std::vector<RenderPassStats>& gpuStats);

        const std::map<StatCategory, CpuRenderStats>& GetCpuStats(uint32_t frameIndex) const;
        const std::vector<RenderPassStats>& GetGpuStats(uint32_t frameIndex) const;
    protected:
        void RecordReadback(VkCommandBuffer cmd, uint32_t frameIndex, Scene* scene);
    private:
        RenderBuffer _readbackBuffer;

        std::vector<std::map<StatCategory, CpuRenderStats>> _cpuStatsPerFrame;
        std::vector<std::vector<RenderPassStats>> _gpuStatsPerFrame;
    };
}