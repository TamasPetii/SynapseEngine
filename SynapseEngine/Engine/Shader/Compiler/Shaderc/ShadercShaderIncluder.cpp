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

#include "ShadercShaderIncluder.h"
#include <filesystem>
#include <fstream>
#include <string>

namespace Syn {
    static std::string ReadFileContent(const std::filesystem::path& path) {
        std::ifstream file(path);
        if (!file.is_open()) return "";
        return std::string((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
    }

    shaderc_include_result* ShadercShaderIncluder::GetInclude(
        const char* requested_source,
        shaderc_include_type type,
        const char* requesting_source,
        size_t include_depth
    ) {
        std::filesystem::path requestPath = requested_source;
        std::filesystem::path currentDir = std::filesystem::path(requesting_source).parent_path();
        std::filesystem::path finalPath = currentDir / requestPath;

        if (!std::filesystem::exists(finalPath)) {
            finalPath = std::filesystem::path("Assets/Shaders") / requestPath;
        }

        if (!std::filesystem::exists(finalPath)) {
            finalPath = std::filesystem::path("Engine/Shaders") / requestPath;
        }

        std::string content = ReadFileContent(finalPath);

        if (content.empty() && !std::filesystem::exists(finalPath)) {
            std::string err = "Could not find include file: " + requestPath.string() + " (Searched relative, Assets/Shaders, and Engine/Shaders)";
            return new shaderc_include_result{ "", 0, err.c_str(), err.length(), nullptr };
        }

        auto* container = new std::pair<std::string, std::string>{ finalPath.string(), content };

        return new shaderc_include_result{
            container->first.c_str(),
            container->first.length(),
            container->second.c_str(),
            container->second.length(),
            container
        };
    }

    void ShadercShaderIncluder::ReleaseInclude(shaderc_include_result* data) {
        delete static_cast<std::pair<std::string, std::string>*>(data->user_data);
        delete data;
    }
}