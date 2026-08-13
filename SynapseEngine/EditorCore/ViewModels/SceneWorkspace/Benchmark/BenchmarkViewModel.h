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