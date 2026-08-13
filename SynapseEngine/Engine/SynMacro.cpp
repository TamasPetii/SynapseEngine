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

#include "SynMacro.h"
#include "Logger/SynLog.h"
#include <format>
#include <iostream>

namespace Syn {

    static void LogAndAbort(const std::string& formattedMsg, const char* file, int line) {
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