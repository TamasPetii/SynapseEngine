#pragma once
#include "EditorCore/ViewModels/IViewModel.h"
#include "Engine/System/SystemPhaseNames.h"
#include "BenchmarkState.h"
#include "BenchmarkIntent.h"

namespace Syn {
    class BenchmarkViewModel : public IViewModel<BenchmarkState, BenchmarkIntent> {
    public:
        BenchmarkViewModel() = default;
        ~BenchmarkViewModel() override = default;

        const BenchmarkState& GetState() const override;

        void SyncWithEngine() override;
        void Dispatch(const BenchmarkIntent& intent) override;
    private:
        std::vector<UiProfilerGroup> ProcessTimings(const std::vector<GroupTiming>& rawTimings, bool parsePhases);
        float CalculateGlobalTotal(const std::vector<GroupTiming>& rawTimings) const;
    private:
        BenchmarkState _state;
        std::unordered_map<std::string, float> _smoothedTimes;
    };
}