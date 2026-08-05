#include "ShaderManager.h"
#include "Engine/Vk/Shader/ShaderUtils.h"
#include "Engine/ServiceLocator.h"
#include "Engine/Logger/SynLog.h"

namespace Syn 
{
    static std::string GenerateShaderKey(const std::string& filepath, std::span<const std::string> defines) {
        std::string key = filepath;
        for (const auto& def : defines) {
            key += "|" + def;
        }
        return key;
    }

    ShaderManager::ShaderManager(std::shared_ptr<ShaderBuilder> builder)
        : _builder(std::move(builder))
    {}

    std::shared_ptr<Shader> ShaderManager::LoadShaderCPU(const std::string& filepath, VkShaderStageFlagBits stage, std::span<const std::string> defines) {
        std::string key = GenerateShaderKey(filepath, defines);

        std::lock_guard lock(_shaderCacheMutex);

        auto it = _cpuShaders.find(key);
        if (it != _cpuShaders.end()) {
            return it->second;
        }

        std::vector<std::string> definesVec(defines.begin(), defines.end());
        auto shader = _builder->BuildFromFile(filepath, stage, definesVec);

        if (shader) {
            _cpuShaders[key] = shader;
        }

        return shader;
    }

    std::shared_ptr<Vk::ShaderProgram> ShaderManager::CreateProgramFromFiles(const std::vector<std::string>& shaderFiles, const Vk::ShaderProgramConfig& config) {
        std::vector<Vk::ShaderCreationInfo> creationInfos;
        creationInfos.reserve(shaderFiles.size());

        for (const auto& file : shaderFiles) {
            VkShaderStageFlagBits stage = Vk::ShaderUtils::GetStageFromExtension(file);
            if (stage == 0) {
                SYN_ASSERT(false, std::string("Unknown shader extension: " + file).c_str());
                continue;
            }

            auto shader = LoadShaderCPU(file, stage, config.defines);
            if (shader && shader->transientGpuData) {
                creationInfos.push_back({
                    .stage = stage,
                    .spirv = &shader->transientGpuData->spirv,
                    .reflection = &shader->cpuData.reflection,
                    .outHandle = &shader->handle
                    });
            }
        }

        return std::make_shared<Vk::ShaderProgram>(creationInfos, config);
    }

    uint32_t ShaderManager::LoadProgramAsync(const std::string& programName, const std::vector<std::string>& shaderFiles, const Vk::ShaderProgramConfig& config) {
        return InternalLoadAsync(programName, [this, shaderFiles, config]() {
            return CreateProgramFromFiles(shaderFiles, config);
            });
    }

    uint32_t ShaderManager::LoadProgramSync(const std::string& programName, const std::vector<std::string>& shaderFiles, const Vk::ShaderProgramConfig& config) {
        return InternalLoadSync(programName, [this, shaderFiles, config]() {
            return CreateProgramFromFiles(shaderFiles, config);
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

    void ShaderManager::FinalizeResource(EntryType& entry) {
    }

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
                return true;
            }
        }
        return false;
    }
}