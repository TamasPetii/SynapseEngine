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