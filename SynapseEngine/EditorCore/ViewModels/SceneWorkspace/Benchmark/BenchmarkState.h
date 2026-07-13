#pragma once
#include "Engine/Profiler/IProfiler.h"
#include <vector>
#include <array>
#include <string>

namespace Syn 
{
    enum class ProfilerTab {
        CPU,
        GPU
    };

    struct UiProfilerEntry {
        std::string name;
        float timeMs = 0.0f;
    };

    struct UiProfilerPhase {
        std::string name;
        float totalTimeMs = 0.0f;
        std::vector<UiProfilerEntry> entries;
    };

    struct UiProfilerGroup {
        std::string name;
        float totalTimeMs = 0.0f;
        std::vector<UiProfilerPhase> phases;
    };

    struct ProfilerFilters {
        bool showUpdate = true;
        bool showUploadGPU = true;
        bool showFinish = true;
        std::string searchQuery = "";
    };

    struct BenchmarkState {
        ProfilerTab activeTab = ProfilerTab::CPU;

        static constexpr int FPS_HISTORY_SIZE = 120;
        std::array<float, FPS_HISTORY_SIZE> fpsHistory = { 0.0f };
        int fpsHistoryOffset = 0;
        float currentFps = 0.0f;
        float averageFps = 0.0f;

        std::vector<UiProfilerGroup> cpuTimings;
        std::vector<UiProfilerGroup> gpuTimings;

        float totalCpuTimeMs = 0.0f;
        float totalGpuTimeMs = 0.0f;

        float warningThresholdMs = 0.5f;
        float criticalThresholdMs = 2.0f;

        ProfilerFilters filters;
    };
}