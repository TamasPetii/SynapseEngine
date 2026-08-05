#pragma once
#include "Engine/SynApi.h"
#include "Engine/Vk/Shader/ShaderProgram.h"
#include "Engine/Manager/BaseResourceManager.h"
#include "Engine/Shader/Data/Shader.h"
#include "Engine/Shader/Builder/ShaderBuilder.h"

#include <string>
#include <memory>
#include <vector>
#include <unordered_map>
#include <mutex>
#include <span>

namespace Syn {
    class SYN_API ShaderManager : public BaseResourceManager<Vk::ShaderProgram> {
    public:
        ShaderManager(std::shared_ptr<ShaderBuilder> builder);
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
        std::shared_ptr<Vk::ShaderProgram> CreateProgramFromFiles(const std::vector<std::string>& shaderFiles, const Vk::ShaderProgramConfig& config);
        std::shared_ptr<Shader> LoadShaderCPU(const std::string& filepath, VkShaderStageFlagBits stage, std::span<const std::string> defines = {});
    private:
        std::mutex _shaderCacheMutex;
        std::shared_ptr<ShaderBuilder> _builder;
        std::unordered_map<std::string, std::shared_ptr<Shader>> _cpuShaders;
    };
}