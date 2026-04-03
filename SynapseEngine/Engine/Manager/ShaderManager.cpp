#include "ShaderManager.h"
#include "Engine/Vk/Shader/ShaderUtils.h"
#include <iostream>
#include <filesystem>

namespace Syn {

    Vk::Shader* ShaderManager::LoadShader(const std::string& filepath) {
        auto it = _shaders.find(filepath);
        if (it != _shaders.end()) {
            return it->second.get();
        }

        VkShaderStageFlagBits stage = Vk::ShaderUtils::GetStageFromExtension(filepath);
        if (stage == 0) {
            SYN_ASSERT(false, std::string("Unknown shader extension: " + filepath).c_str());
            return nullptr;
        }

        return LoadShader(filepath, stage);
    }

    Vk::Shader* ShaderManager::LoadShader(const std::string& filepath, VkShaderStageFlagBits stage) {
        auto it = _shaders.find(filepath);
        if (it != _shaders.end()) {
            return it->second.get();
        }

        auto shader = std::make_unique<Vk::Shader>(filepath, stage);
        Vk::Shader* ptr = shader.get();

        _shaders[filepath] = std::move(shader);

        return ptr;
    }

    Vk::ShaderProgram* ShaderManager::CreateProgram(const std::string& programName, const std::vector<std::string>& shaderFiles, const Vk::ShaderProgramConfig& config) {
        auto it = _programs.find(programName);
        if (it != _programs.end()) {
            return it->second.get();
        }

        std::vector<const Vk::Shader*> shadersForProgram;
        shadersForProgram.reserve(shaderFiles.size());

        for (const auto& file : shaderFiles) {
            Vk::Shader* shader = LoadShader(file);
            if (shader) {
                shadersForProgram.push_back(shader);
            }
        }

        auto program = std::make_unique<Vk::ShaderProgram>(shadersForProgram, config);
        Vk::ShaderProgram* ptr = program.get();

        _programs[programName] = std::move(program);

        return ptr;
    }

    Vk::ShaderProgram* ShaderManager::GetProgram(const std::string& programName) const {
        auto it = _programs.find(programName);
        if (it != _programs.end()) {
            return it->second.get();
        }
        return nullptr;
    }

    Vk::Shader* ShaderManager::GetShader(const std::string& filepath) const  {
        auto it = _shaders.find(filepath);
        if (it != _shaders.end()) {
            return it->second.get();
        }
        return nullptr;
    }

    void ShaderManager::Clear() {
        _programs.clear();
        _shaders.clear();
    }
}