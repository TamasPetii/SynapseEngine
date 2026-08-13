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
#include "EditorCore/Api/ILoggerApi.h"
#include "LoggerState.h"
#include "LoggerIntent.h"

namespace Syn {
    class LoggerViewModel : public IViewModel<LoggerState, LoggerIntent> {
    public:
        explicit LoggerViewModel(ILoggerApi* api);
        ~LoggerViewModel() override = default;

        const LoggerState& GetState() const override;

        void SyncWithEngine() override;
        void Dispatch(const LoggerIntent& intent) override;
    private:
        void ApplyFilters(const std::vector<LogMessage>& rawLogs);
    private:
        ILoggerApi* _api = nullptr;
        LoggerState _state;

        size_t _lastLogCount = 0;
        bool _filtersDirty = true;
    };
}