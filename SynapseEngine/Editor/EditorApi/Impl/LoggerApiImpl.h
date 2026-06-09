#pragma once
#include "EditorCore/Api/ILoggerApi.h"
#include "Engine/Engine.h"

namespace Syn {
    class LoggerApiImpl : public ILoggerApi {
    public:
        LoggerApiImpl(Engine* engine) : _engine(engine) {}
        const std::vector<LogMessage>& GetLogs() const override;
        void ClearLogs() override;
    private:
        Engine* _engine;
    };
}