#include "EditorApiImpl.h"

namespace Syn 
{
    const std::vector<LogMessage>& EditorApiImpl::GetLogs() const {
        return _engine->GetMemorySink()->GetLogs();
    }

    void EditorApiImpl::ClearLogs() {
        _engine->GetMemorySink()->Clear();
    }

}