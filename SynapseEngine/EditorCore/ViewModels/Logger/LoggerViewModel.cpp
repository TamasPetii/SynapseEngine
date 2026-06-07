#include "LoggerViewModel.h"
#include <algorithm>
#include <cctype>

namespace Syn {

    LoggerViewModel::LoggerViewModel(ILoggerApi* api) : _api(api) {}

    const LoggerState& LoggerViewModel::GetState() const {
        return _state;
    }

    void LoggerViewModel::SyncWithEngine() {
        if (!_api) return;

        const auto& rawLogs = _api->GetLogs();

        if (rawLogs.size() != _lastLogCount || _filtersDirty) {
            ApplyFilters(rawLogs);
            _lastLogCount = rawLogs.size();
            _filtersDirty = false;
        }
    }

    void LoggerViewModel::Dispatch(const LoggerIntent& intent) {
        std::visit([this](auto&& arg) {
            using T = std::decay_t<decltype(arg)>;

            if constexpr (std::is_same_v<T, LoggerToggleLevelIntent>) {
                if (arg.level == LogLevel::Info) _state.filters.showInfo = arg.isVisible;
                else if (arg.level == LogLevel::Warning) _state.filters.showWarning = arg.isVisible;
                else if (arg.level == LogLevel::Error) _state.filters.showError = arg.isVisible;
                else if (arg.level == LogLevel::Critical) _state.filters.showCritical = arg.isVisible;
                _filtersDirty = true;
            }
            else if constexpr (std::is_same_v<T, LoggerSetSearchQueryIntent>) {
                _state.filters.searchQuery = arg.query;
                _filtersDirty = true;
            }
            else if constexpr (std::is_same_v<T, LoggerSetAutoScrollIntent>) {
                _state.autoScroll = arg.autoScroll;
            }
            else if constexpr (std::is_same_v<T, LoggerClearIntent>) {
                if (_api) _api->ClearLogs();
                _state.filteredLogs.clear();
                _lastLogCount = 0;
                _filtersDirty = true;
            }
            }, intent);
    }

    void LoggerViewModel::ApplyFilters(const std::vector<LogMessage>& rawLogs) {
        _state.filteredLogs.clear();
        _state.filteredLogs.reserve(rawLogs.size());

        std::string searchLower = _state.filters.searchQuery;
        std::transform(searchLower.begin(), searchLower.end(), searchLower.begin(), [](unsigned char c) { return std::tolower(c); });

        for (const auto& log : rawLogs) {

            if (log.level == LogLevel::Info && !_state.filters.showInfo) continue;
            if (log.level == LogLevel::Warning && !_state.filters.showWarning) continue;
            if (log.level == LogLevel::Error && !_state.filters.showError) continue;
            if (log.level == LogLevel::Critical && !_state.filters.showCritical) continue;

            if (!searchLower.empty()) {
                std::string msgLower(log.message);
                std::string fileLower(log.file);
                std::transform(msgLower.begin(), msgLower.end(), msgLower.begin(), [](unsigned char c) { return std::tolower(c); });
                std::transform(fileLower.begin(), fileLower.end(), fileLower.begin(), [](unsigned char c) { return std::tolower(c); });

                if (msgLower.find(searchLower) == std::string::npos && fileLower.find(searchLower) == std::string::npos) {
                    continue;
                }
            }

            _state.filteredLogs.push_back(log);
        }
    }
}