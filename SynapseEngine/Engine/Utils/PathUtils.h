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

#pragma once
#include "Engine/SynApi.h"
#include <filesystem>
#include <string>

#ifndef SYN_PROJECT_ROOT
#define SYN_PROJECT_ROOT "."
#endif

namespace Syn {
    class SYN_API PathUtils {
    public:
        static inline std::filesystem::path GetAbsolutePath(const std::filesystem::path& path) {
            if (path.is_absolute()) return path.lexically_normal();
            std::filesystem::path root(SYN_PROJECT_ROOT);
            return (root / path).lexically_normal();
        }

        static inline std::string GetAbsolutePathString(const std::string& path) {
            return GetAbsolutePath(path).string();
        }
    };
}