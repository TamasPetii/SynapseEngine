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

#include "StatisticsViewModel.h"
#include "Engine/ServiceLocator.h"
#include "Engine/FrameContext.h"
#include "Engine/Render/PassGroupNames.h"
#include <algorithm>
#include <map>
#include <cctype>

namespace Syn {
    const StatisticsState& StatisticsViewModel::GetState() const {
        return _state;
    }

    void StatisticsViewModel::SyncWithEngine() {
        auto frameCtx = ServiceLocator::Get<FrameContext>();
        auto statsManager = ServiceLocator::Get<FrameStatisticsManager>();

        if (!frameCtx || !statsManager) return;

        uint32_t prevFrame = (frameCtx->currentFrameIndex + frameCtx->framesInFlight - 1) % frameCtx->framesInFlight;

        _lastRawCpuStats = statsManager->GetCpuStats(prevFrame);
        _lastRawStats = statsManager->GetGpuStats(prevFrame);

        ProcessStats(_lastRawStats);
    }

    void StatisticsViewModel::ProcessStats(std::span<const RenderPassStats> rawStats) {
        _state.totalInputVertices = 0;
        _state.totalInputPrimitives = 0;
        _state.totalVSInvocations = 0;
        _state.totalFSInvocations = 0;
        _state.totalMSInvocations = 0;
        _state.totalTSInvocations = 0;
        _state.totalClippingInvocations = 0;
        _state.totalClippingPrimitives = 0;

        _state.cpuStats.totalModels = _lastRawCpuStats.totalModels;
        _state.cpuStats.traditionalDrawDescriptors = _lastRawCpuStats.activeTraditionalCount;
        _state.cpuStats.meshletDrawDescriptors = _lastRawCpuStats.activeMeshletCount;
        _state.cpuStats.totalDrawDescriptors = _state.cpuStats.traditionalDrawDescriptors + _state.cpuStats.meshletDrawDescriptors;
        _state.cpuStats.totalAllocatedInstances = _lastRawCpuStats.totalAllocatedInstances;
        _state.cpuStats.totalMaxMeshlets = _lastRawCpuStats.totalMaxMeshletInstances;
        _state.cpuStats.maxPossibleIndices = _lastRawCpuStats.maxPossibleIndices;

        if (_state.activeTab == StatCategory::All) {
            _state.cpuStats.totalLights = _lastRawCpuStats.totalDirLights + _lastRawCpuStats.totalSpotLights + _lastRawCpuStats.totalPointLights;
            _state.cpuStats.visibleLights = _lastRawCpuStats.visibleDirLights + _lastRawCpuStats.visibleSpotLights + _lastRawCpuStats.visiblePointLights;
            _state.cpuStats.visibleShadowLights = _lastRawCpuStats.shadowDirLights + _lastRawCpuStats.shadowSpotLights + _lastRawCpuStats.shadowPointLights;
            _state.cpuStats.appendedInstances = _lastRawCpuStats.appendedDirInstances + _lastRawCpuStats.appendedSpotInstances + _lastRawCpuStats.appendedPointInstances;
            _state.cpuStats.maxPossibleVertices = _lastRawCpuStats.maxPossibleVertices + _lastRawCpuStats.maxDirVertices + _lastRawCpuStats.maxSpotVertices + _lastRawCpuStats.maxPointVertices;
            _state.cpuStats.maxPossibleTriangles = _lastRawCpuStats.maxPossibleTriangles + _lastRawCpuStats.maxDirTriangles + _lastRawCpuStats.maxSpotTriangles + _lastRawCpuStats.maxPointTriangles;
        }
        else if (_state.activeTab == StatCategory::Scene) {
            _state.cpuStats.totalLights = _lastRawCpuStats.totalDirLights + _lastRawCpuStats.totalSpotLights + _lastRawCpuStats.totalPointLights;
            _state.cpuStats.visibleLights = _lastRawCpuStats.visibleDirLights + _lastRawCpuStats.visibleSpotLights + _lastRawCpuStats.visiblePointLights;
            _state.cpuStats.visibleShadowLights = _lastRawCpuStats.shadowDirLights + _lastRawCpuStats.shadowSpotLights + _lastRawCpuStats.shadowPointLights;
            _state.cpuStats.appendedInstances = 0;
            _state.cpuStats.maxPossibleVertices = _lastRawCpuStats.maxPossibleVertices;
            _state.cpuStats.maxPossibleTriangles = _lastRawCpuStats.maxPossibleTriangles;
        }
        else if (_state.activeTab == StatCategory::DirectionalShadow) {
            _state.cpuStats.totalLights = _lastRawCpuStats.totalDirLights;
            _state.cpuStats.visibleLights = _lastRawCpuStats.visibleDirLights;
            _state.cpuStats.visibleShadowLights = _lastRawCpuStats.shadowDirLights;
            _state.cpuStats.appendedInstances = _lastRawCpuStats.appendedDirInstances;
            _state.cpuStats.maxPossibleVertices = _lastRawCpuStats.maxDirVertices;
            _state.cpuStats.maxPossibleTriangles = _lastRawCpuStats.maxDirTriangles;
        }
        else if (_state.activeTab == StatCategory::SpotShadow) {
            _state.cpuStats.totalLights = _lastRawCpuStats.totalSpotLights;
            _state.cpuStats.visibleLights = _lastRawCpuStats.visibleSpotLights;
            _state.cpuStats.visibleShadowLights = _lastRawCpuStats.shadowSpotLights;
            _state.cpuStats.appendedInstances = _lastRawCpuStats.appendedSpotInstances;
            _state.cpuStats.maxPossibleVertices = _lastRawCpuStats.maxSpotVertices;
            _state.cpuStats.maxPossibleTriangles = _lastRawCpuStats.maxSpotTriangles;
        }
        else if (_state.activeTab == StatCategory::PointShadow) {
            _state.cpuStats.totalLights = _lastRawCpuStats.totalPointLights;
            _state.cpuStats.visibleLights = _lastRawCpuStats.visiblePointLights;
            _state.cpuStats.visibleShadowLights = _lastRawCpuStats.shadowPointLights;
            _state.cpuStats.appendedInstances = _lastRawCpuStats.appendedPointInstances;
            _state.cpuStats.maxPossibleVertices = _lastRawCpuStats.maxPointVertices;
            _state.cpuStats.maxPossibleTriangles = _lastRawCpuStats.maxPointTriangles;
        }

        std::map<std::string, UiStatGroup> groupMap;

        for (const auto& pass : rawStats) {
            bool isDir = (pass.groupName == PassGroupNames::DirectionLightShadowPasses ||
                pass.groupName == PassGroupNames::DirectionLightShadowCullingPasses ||
                pass.groupName == PassGroupNames::DirectionalLightCullingPasses);

            bool isSpot = (pass.groupName == PassGroupNames::SpotLightShadowPasses ||
                pass.groupName == PassGroupNames::SpotLightCullingPasses);

            bool isPoint = (pass.groupName == PassGroupNames::PointLightShadowPasses ||
                pass.groupName == PassGroupNames::PointLightCullingPasses);

            bool isScene = !isDir && !isSpot && !isPoint;

            bool isMatch = false;
            if (_state.activeTab == StatCategory::All) isMatch = true;
            else if (_state.activeTab == StatCategory::DirectionalShadow && isDir) isMatch = true;
            else if (_state.activeTab == StatCategory::SpotShadow && isSpot) isMatch = true;
            else if (_state.activeTab == StatCategory::PointShadow && isPoint) isMatch = true;
            else if (_state.activeTab == StatCategory::Scene && isScene) isMatch = true;

            if (!isMatch) continue;

            bool matchesSearch = _state.searchQuery.empty();
            if (!matchesSearch) {
                std::string fullStr = pass.groupName + " " + pass.passName;
                matchesSearch = std::search(fullStr.begin(), fullStr.end(), _state.searchQuery.begin(), _state.searchQuery.end(),
                    [](char c1, char c2) { return std::tolower(static_cast<unsigned char>(c1)) == std::tolower(static_cast<unsigned char>(c2)); }) != fullStr.end();
            }

            if (!matchesSearch) continue;

            _state.totalInputVertices += pass.inputAssemblyVertices;
            _state.totalInputPrimitives += pass.inputAssemblyPrimitives;
            _state.totalVSInvocations += pass.vertexShaderInvocations;
            _state.totalFSInvocations += pass.fragmentShaderInvocations;
            _state.totalMSInvocations += pass.meshShaderInvocations;
            _state.totalTSInvocations += pass.taskShaderInvocations;
            _state.totalClippingInvocations += pass.clippingInvocations;
            _state.totalClippingPrimitives += pass.clippingPrimitives;

            auto& group = groupMap[pass.groupName];
            group.name = pass.groupName;
            group.inputAssemblyVertices += pass.inputAssemblyVertices;
            group.inputAssemblyPrimitives += pass.inputAssemblyPrimitives;
            group.vertexShaderInvocations += pass.vertexShaderInvocations;
            group.fragmentShaderInvocations += pass.fragmentShaderInvocations;
            group.taskShaderInvocations += pass.taskShaderInvocations;
            group.meshShaderInvocations += pass.meshShaderInvocations;
            group.clippingInvocations += pass.clippingInvocations;
            group.clippingPrimitives += pass.clippingPrimitives;
            group.passes.push_back(pass);
        }

        _state.groupedGpuStats.clear();
        for (auto& [name, group] : groupMap) {
            std::sort(group.passes.begin(), group.passes.end(), [](const auto& a, const auto& b) {
                return a.passName < b.passName;
                });
            _state.groupedGpuStats.push_back(group);
        }
    }

    void StatisticsViewModel::Dispatch(const StatisticsIntent& intent) {
        std::visit([this](auto&& arg) {
            using T = std::decay_t<decltype(arg)>;
            if constexpr (std::is_same_v<T, StatisticsSetSearchQueryIntent>) {
                _state.searchQuery = arg.query;
                ProcessStats(_lastRawStats);
            }
            else if constexpr (std::is_same_v<T, StatisticsSwitchTabIntent>) {
                _state.activeTab = arg.tab;
                ProcessStats(_lastRawStats);
            }
            }, intent);
    }
}