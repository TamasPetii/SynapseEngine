#pragma once
#include "Engine/SynApi.h"
#include "Engine/Statistics/IRenderStatCollector.h"
#include "Engine/Utils/RenderBuffer.h"
#include <vector>
#include <string>

namespace Syn
{
    class Scene;

    struct SYN_API RawCpuRenderStats {
        uint32_t totalModels = 0;
        uint32_t activeTraditionalCount = 0;
        uint32_t activeMeshletCount = 0;
        uint32_t totalAllocatedInstances = 0;
        uint32_t totalMaxMeshletInstances = 0;
        uint64_t maxPossibleVertices = 0;
        uint64_t maxPossibleIndices = 0;
        uint64_t maxPossibleTriangles = 0;

        uint32_t totalDirLights = 0;
        uint32_t totalSpotLights = 0;
        uint32_t totalPointLights = 0;

        uint32_t visibleDirLights = 0;
        uint32_t visibleSpotLights = 0;
        uint32_t visiblePointLights = 0;

        uint32_t shadowDirLights = 0;
        uint32_t shadowSpotLights = 0;
        uint32_t shadowPointLights = 0;

        uint32_t appendedDirInstances = 0;
        uint32_t appendedSpotInstances = 0;
        uint32_t appendedPointInstances = 0;

        uint64_t maxDirVertices = 0;
        uint64_t maxSpotVertices = 0;
        uint64_t maxPointVertices = 0;

        uint64_t maxDirTriangles = 0;
        uint64_t maxSpotTriangles = 0;
        uint64_t maxPointTriangles = 0;
    };

    struct SYN_API GpuCullingReadback {
        uint32_t spotVisibleLights;
        uint32_t spotVisibleShadowLights;
        uint32_t pointVisibleLights;
        uint32_t pointVisibleShadowLights;
        uint32_t spotVisibleShadowInstances;
        uint32_t pointVisibleShadowInstances;
    };

    class SYN_API FrameStatisticsManager {
    public:
        FrameStatisticsManager(uint32_t framesInFlight);

        void ResolveFrame(VkCommandBuffer cmd, Scene* scene, uint32_t frameIndex, const std::vector<RenderPassStats>& gpuStats);

        const RawCpuRenderStats& GetCpuStats(uint32_t frameIndex) const;
        const std::vector<RenderPassStats>& GetGpuStats(uint32_t frameIndex) const;
    protected:
        void RecordReadback(VkCommandBuffer cmd, uint32_t frameIndex, Scene* scene);
    private:
        RenderBuffer _readbackBuffer;

        std::vector<RawCpuRenderStats> _cpuStatsPerFrame;
        std::vector<std::vector<RenderPassStats>> _gpuStatsPerFrame;
    };
}