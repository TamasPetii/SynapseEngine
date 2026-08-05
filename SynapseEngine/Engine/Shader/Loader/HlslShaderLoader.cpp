#include "HlslShaderLoader.h"
#include <fstream>
#include "Engine/Logger/SynLog.h"

namespace Syn {
    std::optional<RawShader> HlslShaderLoader::LoadFile(const std::filesystem::path& path, VkShaderStageFlagBits stage, const std::vector<std::string>& defines) {
        std::ifstream file(path, std::ios::ate | std::ios::binary);
        if (!file.is_open()) {
            Error("Failed to open hlsl shader file: {}", path.string());
            return std::nullopt;
        }

        size_t fileSize = static_cast<size_t>(file.tellg());
        std::vector<uint8_t> buffer(fileSize);
        file.seekg(0);
        file.read(reinterpret_cast<char*>(buffer.data()), fileSize);
        file.close();

        return LoadMemory(buffer, path.string(), stage, defines);
    }

    std::optional<RawShader> HlslShaderLoader::LoadMemory(const std::vector<uint8_t>& data, const std::string& identifier, VkShaderStageFlagBits stage, const std::vector<std::string>& defines) {
        RawShader raw;
        raw.sourceCode = std::string(data.begin(), data.end());
        raw.identifier = identifier;
        raw.stage = stage;
        raw.language = ShaderLanguage::HLSL;
        raw.defines = defines;

        return raw;
    }

    std::vector<std::string> HlslShaderLoader::GetSupportedExtensions() const {
        return { ".hlsl" };
    }
}