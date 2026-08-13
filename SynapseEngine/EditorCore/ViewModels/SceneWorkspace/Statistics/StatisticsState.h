// Copyright (C) 2026 Tamás Péter
// This file is part of SynapseEngine.
//
// SynapseEngine is free software: you can redistribute it and/or modify
// it under the terms of the GNU Affero General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// SynapseEngine is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License
// along with SynapseEngine. If not, see <https://www.gnu.org/licenses/>.

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