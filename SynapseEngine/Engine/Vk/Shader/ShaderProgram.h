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
#include "../VkCommon.h"
#include "ShaderReflectionData.h"
#include <memory>
#include <functional>
#include <vector>
#include <span>
#include <string>

namespace Syn::Vk {

    using DescriptorLayoutOverride = std::function<VkDescriptorSetLayout(uint32_t setIndex)>;

    struct ShaderProgramConfig {
        DescriptorLayoutOverride layoutOverride = nullptr;
        bool useDescriptorBuffers = false;
        std::vector<std::string> defines;
    };

    struct ShaderCreationInfo 
    {
        VkShaderStageFlagBits stage;
        const std::vector<uint32_t>* spirv;
        const ShaderResources* reflection;
        VkShaderEXT* outHandle = nullptr;
    };

    class SYN_API ShaderProgram {
    public:
        ShaderProgram(std::span<const ShaderCreationInfo> creationInfos, const ShaderProgramConfig& config = {});
        ~ShaderProgram();

        ShaderProgram(const ShaderProgram&) = delete;
        ShaderProgram& operator=(const ShaderProgram&) = delete;

        void Bind(VkCommandBuffer cmd) const;
        void CreatePipelineLayoutAndShaders();

        VkPipelineLayout GetLayout() const { return _pipelineLayout; }
    private:
        VkPipelineLayout _pipelineLayout = VK_NULL_HANDLE;
        ShaderProgramConfig _config;

        std::vector<VkShaderEXT> _shaderObjects;
        std::vector<VkShaderStageFlagBits> _stages;
        std::vector<VkDescriptorSetLayout> _createdLayouts;
        std::vector<VkDescriptorSetLayout> _bindLayouts;
        std::vector<ShaderCreationInfo> _pendingCreationInfo;
    };
}