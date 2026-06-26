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