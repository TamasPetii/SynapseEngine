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

#include "FileSink.h"
#include "Engine/Logger/LogUtils.h"
#include <filesystem>
#include <format>

namespace Syn
{
    FileSink::FileSink() {
        const char* appDataPath = std::getenv("APPDATA");
        std::filesystem::path baseDir = appDataPath ? appDataPath : ".";
        std::filesystem::path logDir = baseDir / "Synapse" / "Logs";

        if (!std::filesystem::exists(logDir)) {
            std::filesystem::create_directories(logDir);
        }

        std::string filename = std::format("{}/SynapseEngine_{}.log",
            logDir.string(),
            Syn::LogUtils::GetCurrentTimeForFileName());

        _file.open(filename, std::ios::out);
    }

    void FileSink::Log(const LogMessage& msg) {
        std::scoped_lock lock(_mtx);

        if (_file.is_open()) {
            _file << msg.ToString() << "\n";
            _file.flush();
        }
    }
}