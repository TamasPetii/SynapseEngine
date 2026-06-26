#include "LoggerApiImpl.h"

namespace Syn {
    const std::vector<LogMessage>& LoggerApiImpl::GetLogs() const {
        return _engine->GetMemorySink()->GetLogs();
    }

    void LoggerApiImpl::ClearLogs() {
        _engine->GetMemorySink()->Clear();
    }
}