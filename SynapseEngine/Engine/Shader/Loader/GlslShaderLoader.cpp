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

#include "GlslShaderLoader.h"
#include <fstream>
#include "Engine/Logger/SynLog.h"

namespace Syn {
    std::optional<RawShader> GlslShaderLoader::LoadFile(const std::filesystem::path& path, VkShaderStageFlagBits stage, const std::vector<std::string>& defines) {
        std::ifstream file(path, std::ios::ate | std::ios::binary);
        if (!file.is_open()) {
            Error("Failed to open shader file: {}", path.string());
            return std::nullopt;
        }

        size_t fileSize = static_cast<size_t>(file.tellg());
        std::vector<uint8_t> buffer(fileSize);
        file.seekg(0);
        file.read(reinterpret_cast<char*>(buffer.data()), fileSize);
        file.close();

        return LoadMemory(buffer, path.string(), stage, defines);
    }

    std::optional<RawShader> GlslShaderLoader::LoadMemory(const std::vector<uint8_t>& data, const std::string& identifier, VkShaderStageFlagBits stage, const std::vector<std::string>& defines) {
        RawShader raw;
        raw.sourceCode = std::string(data.begin(), data.end());
        raw.identifier = identifier;
        raw.stage = stage;
        raw.language = ShaderLanguage::GLSL;
        raw.defines = defines;

        return raw;
    }

    std::vector<std::string> GlslShaderLoader::GetSupportedExtensions() const {
        return { ".glsl", ".vert", ".frag", ".comp", ".geom", ".tesc", ".tese", ".mesh", ".task", ".rgen", ".rmiss", ".rchit", ".rahit", ".rint", ".rcall" };
    }
}