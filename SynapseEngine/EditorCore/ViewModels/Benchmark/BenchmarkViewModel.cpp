#include "BenchmarkViewModel.h"
#include "Engine/ServiceLocator.h"
#include "Engine/FrameContext.h"
#include "Engine/Profiler/IProfiler.h"
#include "Engine/Profiler/ICpuProfiler.h"
#include "Engine/Profiler/IGpuProfiler.h"
#include <algorithm>

namespace Syn {

    const BenchmarkState& BenchmarkViewModel::GetState() const {
        return _state;
    }

    void BenchmarkViewModel::SyncWithEngine() {
        auto frameCtx = ServiceLocator::GetFrameContext();
        if (!frameCtx) return;

        float fps = frameCtx->deltaTime > 0.0f ? (1.0f / frameCtx->deltaTime) : 0.0f;
        _state.currentFps = fps;
        _state.fpsHistory[_state.fpsHistoryOffset] = fps;
        _state.fpsHistoryOffset = (_state.fpsHistoryOffset + 1) % BenchmarkState::FPS_HISTORY_SIZE;

        float sumFps = 0.0f;
        for (float f : _state.fpsHistory) sumFps += f;
        _state.averageFps = sumFps / BenchmarkState::FPS_HISTORY_SIZE;

        uint32_t prevFrame = (frameCtx->currentFrameIndex + frameCtx->framesInFlight - 1) % frameCtx->framesInFlight;

        if (auto cpuProfiler = ServiceLocator::GetCpuProfiler()) {
            auto rawTimings = cpuProfiler->GetTimings(prevFrame);
            _state.totalCpuTimeMs = CalculateGlobalTotal(rawTimings);
            _state.cpuTimings = ProcessTimings(rawTimings, true);
        }

        if (auto gpuProfiler = ServiceLocator::GetGpuProfiler()) {
            auto rawTimings = gpuProfiler->GetTimings(prevFrame);
            _state.totalGpuTimeMs = CalculateGlobalTotal(rawTimings);
            _state.gpuTimings = ProcessTimings(rawTimings, false);
        }
    }

    float BenchmarkViewModel::CalculateGlobalTotal(const std::vector<GroupTiming>& rawTimings) const {
        float total = 0.0f;
        for (const auto& group : rawTimings) total += group.totalTimeMs;
        return total;
    }

    std::vector<UiProfilerGroup> BenchmarkViewModel::ProcessTimings(const std::vector<GroupTiming>& rawTimings, bool parsePhases) {
        std::vector<UiProfilerGroup> result;

        for (const auto& rawGroup : rawTimings) {
            UiProfilerGroup uiGroup;
            uiGroup.name = rawGroup.name;

            std::unordered_map<std::string, UiProfilerPhase> phaseMap;

            for (const auto& rawEntry : rawGroup.entries) {
                std::string entryName = rawEntry.name;
                std::string phaseName = "";

                if (parsePhases) {
                    size_t bStart = entryName.find('[');
                    size_t bEnd = entryName.find(']');
                    if (bStart != std::string::npos && bEnd != std::string::npos) {
                        phaseName = entryName.substr(bStart + 1, bEnd - bStart - 1);
                        entryName = entryName.substr(0, bStart);
                        entryName.erase(entryName.find_last_not_of(" \n\r\t") + 1);
                    }
                    else {
                        std::vector<std::string> knownPhases = { 
                            SystemPhaseNames::Update,
                            SystemPhaseNames::UploadGPU,
                            SystemPhaseNames::UploadSparseMap,
                            SystemPhaseNames::FinishResetState,
                            SystemPhaseNames::Finish
                        };

                        for (const auto& known : knownPhases) {
                            size_t pos = entryName.find(known);
                            if (pos != std::string::npos) {
                                phaseName = entryName.substr(pos);
                                entryName = entryName.substr(0, pos);
                                entryName.erase(entryName.find_last_not_of(" \n\r\t") + 1);
                                break;
                            }
                        }
                    }
                }

                bool matchesSearch = _state.filters.searchQuery.empty();
                if (!matchesSearch) {
                    std::string fullStr = rawGroup.name + " " + entryName + " " + phaseName;
                    matchesSearch = std::search(fullStr.begin(), fullStr.end(), _state.filters.searchQuery.begin(), _state.filters.searchQuery.end(),
                        [](char c1, char c2) { return std::tolower(static_cast<unsigned char>(c1)) == std::tolower(static_cast<unsigned char>(c2)); }) != fullStr.end();
                }
                if (!matchesSearch) continue;

                if (!_state.filters.showUpdate && phaseName.find(SystemPhaseNames::Update) != std::string::npos) continue;
                if (!_state.filters.showUploadGPU && phaseName.find(SystemPhaseNames::UploadGPU) != std::string::npos) continue; // Catches GPU and Sparse Map
                if (!_state.filters.showFinish && phaseName.find(SystemPhaseNames::Finish) != std::string::npos) continue;

                std::string key = rawGroup.name + "_" + phaseName + "_" + entryName;
                float smoothed = _smoothedTimes[key];
                if (smoothed == 0.0f) smoothed = rawEntry.timeMs;
                smoothed = smoothed * 0.90f + rawEntry.timeMs * 0.10f;
                _smoothedTimes[key] = smoothed;

                phaseMap[phaseName].name = phaseName;
                phaseMap[phaseName].entries.push_back({ entryName, smoothed });
                phaseMap[phaseName].totalTimeMs += smoothed;
            }

            for (auto& [pName, phase] : phaseMap) {
                std::sort(phase.entries.begin(), phase.entries.end(), [](const auto& a, const auto& b) { return a.name < b.name; });
                uiGroup.phases.push_back(phase);
                uiGroup.totalTimeMs += phase.totalTimeMs;
            }

            std::sort(uiGroup.phases.begin(), uiGroup.phases.end(), [](const auto& a, const auto& b) {
                auto getWeight = [](const std::string& n) {
                    if (n.find(SystemPhaseNames::Update) != std::string::npos) return 1;
                    if (n.find(SystemPhaseNames::UploadGPU) != std::string::npos) return 2;
                    if (n.find(SystemPhaseNames::Finish) != std::string::npos) return 3;
                    return 4;
                    };
                return getWeight(a.name) < getWeight(b.name);
                });

            if (!uiGroup.phases.empty()) {
                result.push_back(uiGroup);
            }
        }

        std::sort(result.begin(), result.end(), [](const UiProfilerGroup& a, const UiProfilerGroup& b) {
            return a.name < b.name;
            });

        return result;
    }

    void BenchmarkViewModel::Dispatch(const BenchmarkIntent& intent) {
        std::visit([this](auto&& arg) {
            using T = std::decay_t<decltype(arg)>;
            if constexpr (std::is_same_v<T, BenchmarkSwitchTabIntent>) {
                _state.activeTab = arg.tab;
            }
            else if constexpr (std::is_same_v<T, BenchmarkSetThresholdsIntent>) {
                _state.warningThresholdMs = arg.warning;
                _state.criticalThresholdMs = arg.critical;
            }
            else if constexpr (std::is_same_v<T, BenchmarkSetSearchQueryIntent>) {
                _state.filters.searchQuery = arg.query;
            }
            else if constexpr (std::is_same_v<T, BenchmarkTogglePhaseFilterIntent>) {
                if (arg.phase == SystemPhaseNames::Update) _state.filters.showUpdate = arg.isVisible;
                else if (arg.phase == SystemPhaseNames::UploadGPU) _state.filters.showUploadGPU = arg.isVisible;
                else if (arg.phase == SystemPhaseNames::Finish) _state.filters.showFinish = arg.isVisible;
            }
            }, intent);
    }
}