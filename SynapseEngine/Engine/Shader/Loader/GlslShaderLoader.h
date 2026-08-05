#pragma once
#include "IShaderLoader.h"

namespace Syn {
    class SYN_API GlslShaderLoader : public IShaderLoader {
    public:
        GlslShaderLoader() = default;
        ~GlslShaderLoader() override = default;

        std::optional<RawShader> LoadFile(const std::filesystem::path& path, VkShaderStageFlagBits stage, const std::vector<std::string>& defines) override;
        std::optional<RawShader> LoadMemory(const std::vector<uint8_t>& data, const std::string& identifier, VkShaderStageFlagBits stage, const std::vector<std::string>& defines) override;
        std::vector<std::string> GetSupportedExtensions() const override;
    };
}