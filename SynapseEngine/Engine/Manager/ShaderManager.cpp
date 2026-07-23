#include "ShaderManager.h"
#include "Engine/Vk/Shader/ShaderUtils.h"
#include "Engine/ServiceLocator.h"
#include "Engine/Vk/Context.h"
#include "Engine/Logger/SynLog.h"

namespace Syn {
    static std::string GenerateShaderKey(const std::string& filepath, std::span<const std::string> defines) {
        std::string key = filepath;
        for (const auto& def : defines) {
            key += "|" + def;
        }
        return key;
    }

    const Vk::Shader* ShaderManager::LoadShaderCPU(const std::string& filepath, std::span<const std::string> defines) {
        VkShaderStageFlagBits stage = Vk::ShaderUtils::GetStageFromExtension(filepath);
        if (stage == 0) {
            SYN_ASSERT(false, std::string("Unknown shader extension: " + filepath).c_str());
            return nullptr;
        }

        return LoadShaderCPU(filepath, stage, defines);
    }

    const Vk::Shader* ShaderManager::LoadShaderCPU(const std::string& filepath, VkShaderStageFlagBits stage, std::span<const std::string> defines) {
        std::string key = GenerateShaderKey(filepath, defines);

        std::lock_guard lock(_shaderCacheMutex);

        auto it = _cpuShaders.find(key);
        if (it != _cpuShaders.end()) {
            return it->second.get();
        }

        auto shader = std::make_unique<Vk::Shader>(filepath, stage, defines);
        const Vk::Shader* ptr = shader.get();

        _cpuShaders[key] = std::move(shader);

        return ptr;
    }

    uint32_t ShaderManager::LoadProgramAsync(const std::string& programName, const std::vector<std::string>& shaderFiles, const Vk::ShaderProgramConfig& config) {
        return InternalLoadAsync(programName, [this, shaderFiles, config]() {
            std::vector<const Vk::Shader*> shadersForProgram;
            shadersForProgram.reserve(shaderFiles.size());

            for (const auto& file : shaderFiles) {
                const Vk::Shader* shader = LoadShaderCPU(file, config.defines);
                if (shader) {
                    shadersForProgram.push_back(shader);
                }
            }

            return std::make_shared<Vk::ShaderProgram>(shadersForProgram, config);
            });
    }

    uint32_t ShaderManager::LoadProgramSync(const std::string& programName, const std::vector<std::string>& shaderFiles, const Vk::ShaderProgramConfig& config) {
        return InternalLoadSync(programName, [this, shaderFiles, config]() {
            std::vector<const Vk::Shader*> shadersForProgram;
            shadersForProgram.reserve(shaderFiles.size());

            for (const auto& file : shaderFiles) {
                const Vk::Shader* shader = LoadShaderCPU(file, config.defines);
                if (shader) {
                    shadersForProgram.push_back(shader);
                }
            }
            return std::make_shared<Vk::ShaderProgram>(shadersForProgram, config);
            });
    }

    void ShaderManager::StartGpuUpload(EntryType& entry) {
        uint32_t entryId = GetResourceIndex(entry.path);
        auto program = entry.resource;

        Vk::GpuUploadRequest request{
            .uploadCallback = [program](VkCommandBuffer cmd) {
                program->CreatePipelineLayoutAndShaders();
            },
            .onFinished = [this, entryId]() {
                SetResourceState(entryId, ResourceState::Ready);
                MarkDirty(entryId);
            },
            .needsGraphics = false
        };

        SubmitGpuRequest(entry, std::move(request));
    }

    void ShaderManager::FinalizeResource(EntryType& entry) {}

    void ShaderManager::Clear() {
        std::lock_guard lock(_mutex);
        std::lock_guard cacheLock(_shaderCacheMutex);

        _entries.clear();
        _pathToId.clear();
        _cpuShaders.clear();
        _hasDirty = true;
    }

    bool ShaderManager::IsCompiling() const {
        std::lock_guard lock(_mutex);
        for (const auto& entry : _entries) {
            if (entry.state == ResourceState::LoadingCPU || entry.state == ResourceState::UploadingGPU) {
                std::cout << entry.path << std::endl;
                return true;
            }
        }
        return false;
    }
}