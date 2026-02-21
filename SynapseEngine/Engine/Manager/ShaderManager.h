#pragma once
#include "Engine/Vk/Shader/Shader.h"
#include "Engine/Vk/Shader/ShaderProgram.h"

#include <string>
#include <memory>
#include <vector>
#include <unordered_map>

namespace Syn {
    class SYN_API ShaderManager {
    public:
        ShaderManager() = default;
        ~ShaderManager() = default;

        ShaderManager(const ShaderManager&) = delete;
        ShaderManager& operator=(const ShaderManager&) = delete;

        Vk::Shader* LoadShader(const std::string& filepath);
        Vk::Shader* LoadShader(const std::string& filepath, VkShaderStageFlagBits stage);
        Vk::Shader* GetShader(const std::string& filepath) const;

        Vk::ShaderProgram* CreateProgram(const std::string& programName, const std::vector<std::string>& shaderFiles);
        Vk::ShaderProgram* GetProgram(const std::string& programName) const;

        void Clear();
    private:
        std::unordered_map<std::string, std::unique_ptr<Vk::Shader>> _shaders;
        std::unordered_map<std::string, std::unique_ptr<Vk::ShaderProgram>> _programs;
    };
}