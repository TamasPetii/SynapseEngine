#pragma once
#include "Engine/Statistics/IRenderStatCollector.h"
#include "Engine/Statistics/FrameStatisticsManager.h" 
#include <vector>
#include <string>

namespace Syn
{
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
        StatCategory activeTab = StatCategory::Scene;

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