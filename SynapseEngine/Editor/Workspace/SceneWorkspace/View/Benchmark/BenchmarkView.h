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
#include "Editor/Workspace/IView.h"
#include "EditorCore/ViewModels/SceneWorkspace/Benchmark/BenchmarkViewModel.h"
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