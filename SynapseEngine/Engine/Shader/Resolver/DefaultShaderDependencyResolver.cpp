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

#include "DefaultShaderDependencyResolver.h"
#include "Engine/SynMacro.h"
#include "Engine/Utils/PathUtils.h"
#include <fstream>
#include <regex>

namespace Syn {
    std::unordered_set<std::string> DefaultShaderDependencyResolver::ResolveDependencies(const std::filesystem::path& sourcePath) {
        std::unordered_set<std::string> dependencies;
        GatherDependencies(sourcePath, dependencies);
        return dependencies;
    }

    void DefaultShaderDependencyResolver::GatherDependencies(const std::filesystem::path& currentFile, std::unordered_set<std::string>& dependencies) {
        if (!std::filesystem::exists(currentFile)) return;

        std::string pathStr = currentFile.lexically_normal().string();
        if (dependencies.contains(pathStr)) return;
        dependencies.insert(pathStr);

        std::ifstream file(currentFile);
        if (!file.is_open()) return;

        std::string line;
        std::regex includeRegex(R"x(^[ \t]*#[ \t]*include[ \t]+["<](.*)[">])x");
        std::smatch match;

        while (std::getline(file, line)) {
            if (std::regex_search(line, match, includeRegex)) {
                if (match.size() == 2) {
                    std::filesystem::path includePath = match[1].str();
                    std::filesystem::path currentDir = currentFile.parent_path();

                    std::filesystem::path finalPath = (currentDir / includePath).lexically_normal();

                    if (!std::filesystem::exists(finalPath)) {
                        std::filesystem::path projectRoot(SYN_PROJECT_ROOT);
                        finalPath = (projectRoot / "Engine/Shaders" / includePath).lexically_normal();
                    }

                    if (!std::filesystem::exists(finalPath)) {
                        std::filesystem::path projectRoot(SYN_PROJECT_ROOT);
                        finalPath = (projectRoot / "Assets/Shaders" / includePath).lexically_normal();
                    }

                    GatherDependencies(finalPath, dependencies);
                }
            }
        }
    }
}