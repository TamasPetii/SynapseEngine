#pragma once
#include "Engine/SynApi.h"
#include "Engine/Shader/Data/Raw/RawShader.h"
#include <optional>
#include <filesystem>
#include <string>
#include <vector>

namespace Syn {
    class SYN_API IShaderLoader {
    public:
        virtual ~IShaderLoader() = default;

        virtual std::optional<RawShader> LoadFile(const std::filesystem::path& path, VkShaderStageFlagBits stage, const std::vector<std::string>& defines) = 0;
        virtual std::optional<RawShader> LoadMemory(const std::vector<uint8_t>& data, const std::string& identifier, VkShaderStageFlagBits stage, const std::vector<std::string>& defines) = 0;

        virtual std::vector<std::string> GetSupportedExtensions() const = 0;
    };
}