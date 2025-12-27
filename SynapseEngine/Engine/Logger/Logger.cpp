#include "Logger.h"

namespace Syn {

    Logger& Logger::Get() {
        static Logger instance;
        return instance;
    }

    void Logger::AddSink(std::shared_ptr<ISink> sink) {
        _sinks.push_back(sink);
    }

    void Logger::Dispatch(LogLevel level, std::string_view msg, const char* file, int line) {
        if (!Syn::EnableLogging) return;

        LogMessage payload {
            .level = level,
            .message = msg,
            .file = file,
            .line = line,
            .time = std::chrono::system_clock::now()
        };

        for (auto& sink : _sinks) {
            sink->Log(payload);
        }
    }
}