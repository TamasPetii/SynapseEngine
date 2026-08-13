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

#include "Logger.h"

namespace Syn {

    Logger& Logger::Get() {
        static Logger instance;
        return instance;
    }

    void Logger::AddSink(std::shared_ptr<ISink> sink) {
        _sinks.push_back(sink);
    }

    void Logger::Dispatch(LogLevel level, const std::string& msg, const char* file, int line) {
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