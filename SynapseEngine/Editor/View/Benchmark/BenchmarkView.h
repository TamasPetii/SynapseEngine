#pragma once
#include "Editor/View/IView.h"
#include "EditorCore/ViewModels/Benchmark/BenchmarkViewModel.h"
#include <unordered_map>
#include <string>

namespace Syn {
    class BenchmarkView : public IView<BenchmarkViewModel> {
    public:
        void Draw(BenchmarkViewModel& vm) override;
    private:
        void RenderTopBar(const BenchmarkState& state);
        void RenderFilterBar(BenchmarkViewModel& vm, const BenchmarkState& state);
        void RenderProfilerTable(BenchmarkViewModel& vm, const std::vector<UiProfilerGroup>& timings, float totalTime, const BenchmarkState& state, float mainContentBottomY);
        void RenderGroupRow(const UiProfilerGroup& group, float globalTotalTime, const BenchmarkState& state);
        void RenderProgressBar(float timeMs, float referenceTimeMs, const BenchmarkState& state);
        void ImGuiColorBasedOnTime(float timeMs, const BenchmarkState& state);
    private:
        std::unordered_map<std::string, bool> _cardStates;
    };
}