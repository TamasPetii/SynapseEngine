#include "SynMacro.h"
#include "Logger/SynLog.h"
#include <format>
#include <iostream>

namespace Syn {

    static void LogAndAbort(std::string_view formattedMsg, const char* file, int line) {
        Logger::Get().Dispatch(LogLevel::Error, formattedMsg, file, line);
        std::abort();
    }

    void HandleAssert(const char* msg, const char* file, int line) {
        std::string message = std::format("ASSERTION FAILED: {}", msg);
        LogAndAbort(message, file, line);
    }

    void HandleVkAssert(int result, const char* expr, const char* file, int line) {
        return;

        if (result != 0) {
            std::string message = std::format("VULKAN ERROR: {} (Code: {})", expr, result);
            LogAndAbort(message, file, line);
        }
        else
			Logger::Get().Dispatch(LogLevel::Info, "Vulkan call succeeded: " + std::string(expr), file, line);
    }

    void HandleVkAssertMsg(int result, const char* expr, const char* msg, const char* file, int line) {
        return;
        
        if (result != 0) {
            std::string message = std::format("VULKAN ERROR: {}\n\tExpression: {} (Code: {})", msg, expr, result);
            LogAndAbort(message, file, line);
        }
        else
            Logger::Get().Dispatch(LogLevel::Info, "Vulkan call succeeded: " + std::string(expr), file, line);
    }
}