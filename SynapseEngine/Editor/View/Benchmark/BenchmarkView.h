#pragma once
#include "Editor/View/IView.h"
#include "EditorCore/ViewModels/Benchmark/BenchmarkViewModel.h"

namespace Syn {
    class BenchmarkView : public IView<BenchmarkViewModel> {
    public:
        void Draw(BenchmarkViewModel& vm) override;
    private:
        void RenderTopBar(const BenchmarkState& state);
        void RenderFilterBar(BenchmarkViewModel& vm, const BenchmarkState& state);
        void RenderProfilerTable(const std::vector<UiProfilerGroup>& timings, float totalTime, const BenchmarkState& state);
        void RenderGroupRow(const UiProfilerGroup& group, float globalTotalTime, const BenchmarkState& state);
        void RenderProgressBar(float timeMs, float referenceTimeMs, const BenchmarkState& state);
        void ImGuiColorBasedOnTime(float timeMs, const BenchmarkState& state);
    };
}