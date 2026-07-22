#include "StatisticsViewModel.h"
#include "Engine/ServiceLocator.h"
#include "Engine/FrameContext.h"
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

        _lastCpuStatsMap = statsManager->GetCpuStats(prevFrame);
        _lastRawStats = statsManager->GetGpuStats(prevFrame);

        _state.cpuStats = _lastCpuStatsMap[_state.activeTab];
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

        std::map<std::string, UiStatGroup> groupMap;

        for (const auto& pass : rawStats) {
            std::string lowerGroup = pass.groupName;
            std::transform(lowerGroup.begin(), lowerGroup.end(), lowerGroup.begin(), ::tolower);

            bool isDir = lowerGroup.find("direction") != std::string::npos || lowerGroup.find("dir") != std::string::npos;
            bool isSpot = lowerGroup.find("spot") != std::string::npos;
            bool isPoint = lowerGroup.find("point") != std::string::npos;

            bool isMatch = false;
            if (_state.activeTab == StatCategory::DirectionalShadow && isDir) isMatch = true;
            else if (_state.activeTab == StatCategory::SpotShadow && isSpot) isMatch = true;
            else if (_state.activeTab == StatCategory::PointShadow && isPoint) isMatch = true;
            else if (_state.activeTab == StatCategory::Scene && !isDir && !isSpot && !isPoint) isMatch = true;

            if (!isMatch) continue;

            _state.totalInputVertices += pass.inputAssemblyVertices;
            _state.totalInputPrimitives += pass.inputAssemblyPrimitives;
            _state.totalVSInvocations += pass.vertexShaderInvocations;
            _state.totalFSInvocations += pass.fragmentShaderInvocations;
            _state.totalMSInvocations += pass.meshShaderInvocations;
            _state.totalTSInvocations += pass.taskShaderInvocations;
            _state.totalClippingInvocations += pass.clippingInvocations;
            _state.totalClippingPrimitives += pass.clippingPrimitives;

            bool matchesSearch = _state.searchQuery.empty();
            if (!matchesSearch) {
                std::string fullStr = pass.groupName + " " + pass.passName;
                matchesSearch = std::search(fullStr.begin(), fullStr.end(), _state.searchQuery.begin(), _state.searchQuery.end(),
                    [](char c1, char c2) { return std::tolower(static_cast<unsigned char>(c1)) == std::tolower(static_cast<unsigned char>(c2)); }) != fullStr.end();
            }

            if (!matchesSearch) continue;

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
                _state.cpuStats = _lastCpuStatsMap[_state.activeTab];
                ProcessStats(_lastRawStats);
            }
            }, intent);
    }
}