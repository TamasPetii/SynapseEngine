#pragma once
#include "Engine/Statistics/IRenderStatCollector.h"
#include <vector>
#include <string>

namespace Syn
{
    enum class StatCategory {
        All,
        Scene,
        DirectionalShadow,
        PointShadow,
        SpotShadow
    };

    struct CpuRenderStats {
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

    struct UiStatGroup {
        std::string name;
        uint64_t inputAssemblyVertices = 0;
        uint64_t inputAssemblyPrimitives = 0;
        uint64_t vertexShaderInvocations = 0;
        uint64_t fragmentShaderInvocations = 0;
        uint64_t taskShaderInvocations = 0;
        uint64_t meshShaderInvocations = 0;
        uint64_t clippingInvocations = 0;
        uint64_t clippingPrimitives = 0;
        std::vector<RenderPassStats> passes;
    };

    struct StatisticsState {
        StatCategory activeTab = StatCategory::All;

        CpuRenderStats cpuStats;
        std::vector<UiStatGroup> groupedGpuStats;

        uint64_t totalInputVertices = 0;
        uint64_t totalInputPrimitives = 0;
        uint64_t totalVSInvocations = 0;
        uint64_t totalFSInvocations = 0;
        uint64_t totalMSInvocations = 0;
        uint64_t totalTSInvocations = 0;
        uint64_t totalClippingInvocations = 0;
        uint64_t totalClippingPrimitives = 0;

        std::string searchQuery = "";
    };
}