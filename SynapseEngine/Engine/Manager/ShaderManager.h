#pragma once
#include "Engine/Vk/Shader/Shader.h"
#include "Engine/Vk/Shader/ShaderProgram.h"
#include "BaseResourceManager.h"

#include <string>
#include <memory>
#include <vector>
#include <unordered_map>
#include <mutex>
#include <span>

namespace Syn {
    class SYN_API ShaderManager : public BaseResourceManager<Vk::ShaderProgram> {
    public:
        ShaderManager() = default;
        ~ShaderManager() override = default;

        ShaderManager(const ShaderManager&) = delete;
        ShaderManager& operator=(const ShaderManager&) = delete;

        uint32_t LoadProgramAsync(const std::string& programName, const std::vector<std::string>& shaderFiles, const Vk::ShaderProgramConfig& config = {});
        uint32_t LoadProgramSync(const std::string& programName, const std::vector<std::string>& shaderFiles, const Vk::ShaderProgramConfig& config = {});

        void Clear();
        bool IsCompiling() const;
    protected:
        void StartGpuUpload(EntryType& entry) override;
        void FinalizeResource(EntryType& entry) override;
    private:
        const Vk::Shader* LoadShaderCPU(const std::string& filepath, std::span<const std::string> defines = {});
        const Vk::Shader* LoadShaderCPU(const std::string& filepath, VkShaderStageFlagBits stage, std::span<const std::string> defines = {});

        std::mutex _shaderCacheMutex;
        std::unordered_map<std::string, std::unique_ptr<Vk::Shader>> _cpuShaders;
    };
}