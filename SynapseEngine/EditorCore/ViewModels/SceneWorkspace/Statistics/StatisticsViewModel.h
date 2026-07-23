#pragma once
#include "EditorCore/ViewModels/IViewModel.h"
#include "StatisticsState.h"
#include "StatisticsIntent.h"
#include "Engine/Statistics/FrameStatisticsManager.h"
#include <vector>
#include <map>
#include <span>

namespace Syn {
    class StatisticsViewModel : public IViewModel<StatisticsState, StatisticsIntent> {
    public:
        StatisticsViewModel() = default;
        ~StatisticsViewModel() override = default;

        const StatisticsState& GetState() const override;

        void SyncWithEngine() override;
        void Dispatch(const StatisticsIntent& intent) override;
    private:
        void ProcessStats(std::span<const RenderPassStats> rawStats);

        StatisticsState _state;
        RawCpuRenderStats _lastRawCpuStats;
        std::vector<RenderPassStats> _lastRawStats;
    };
}