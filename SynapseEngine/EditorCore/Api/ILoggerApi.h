#pragma once
#include "IApi.h"
#include "Engine/Logger/LogMessage.h"
#include <span>

namespace Syn {
    class ILoggerApi : public IApi {
    public:
        virtual ~ILoggerApi() = default;
        virtual const std::vector<LogMessage>& GetLogs() const = 0;
        virtual void ClearLogs() = 0;
    };
}