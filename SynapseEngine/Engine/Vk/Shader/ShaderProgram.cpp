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

#include "ShaderProgram.h"
#include "Engine/ServiceLocator.h"
#include "Engine/Vk/Context.h"
#include "Engine/Vk/Descriptor/DescriptorUtils.h"
#include "Engine/Logger/SynLog.h"
#include <map>

namespace Syn::Vk {

    ShaderProgram::ShaderProgram(std::span<const ShaderCreationInfo> creationInfos, const ShaderProgramConfig& config) :
        _config(config)
    {
        _pendingCreationInfo.assign(creationInfos.begin(), creationInfos.end());
    }

    ShaderProgram::~ShaderProgram() {
        auto device = ServiceLocator::Get<Vk::Context>()->GetDevice();

        for (auto shaderObj : _shaderObjects) {
            if (shaderObj != VK_NULL_HANDLE) {
                vkDestroyShaderEXT(device->Handle(), shaderObj, nullptr);
            }
        }

        if (_pipelineLayout != VK_NULL_HANDLE) {
            vkDestroyPipelineLayout(device->Handle(), _pipelineLayout, nullptr);
        }

        for (auto layout : _createdLayouts) {
            if (layout != VK_NULL_HANDLE) {
                vkDestroyDescriptorSetLayout(device->Handle(), layout, nullptr);
            }
        }
    }

    void ShaderProgram::Bind(VkCommandBuffer cmd) const {
        bool isCompute = false;
        for (auto stage : _stages) {
            if (stage == VK_SHADER_STAGE_COMPUTE_BIT) {
                isCompute = true;
                break;
            }
        }

        std::vector<VkShaderStageFlagBits> stagesToBind;
        if (isCompute) {
            stagesToBind = { 
                VK_SHADER_STAGE_COMPUTE_BIT
            };
        }
        else {
            stagesToBind = {
                VK_SHADER_STAGE_VERTEX_BIT,
                VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT,
                VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT,
                VK_SHADER_STAGE_GEOMETRY_BIT,
                VK_SHADER_STAGE_FRAGMENT_BIT,
                VK_SHADER_STAGE_TASK_BIT_EXT,
                VK_SHADER_STAGE_MESH_BIT_EXT
            };
        }

        std::vector<VkShaderEXT> handlesToBind(stagesToBind.size(), VK_NULL_HANDLE);

        for (size_t i = 0; i < _shaderObjects.size(); ++i) {
            VkShaderStageFlagBits currentStage = _stages[i];

            for (size_t j = 0; j < stagesToBind.size(); ++j) {
                if (stagesToBind[j] == currentStage) {
                    handlesToBind[j] = _shaderObjects[i];
                    break;
                }
            }
        }

        vkCmdBindShadersEXT(cmd, static_cast<uint32_t>(stagesToBind.size()), stagesToBind.data(), handlesToBind.data());
    }

    void ShaderProgram::CreatePipelineLayoutAndShaders() {
        if (_pendingCreationInfo.empty()) return;

        auto device = ServiceLocator::Get<Vk::Context>()->GetDevice();
        std::map<uint32_t, std::map<uint32_t, VkDescriptorSetLayoutBinding>> mergedBindings;

        for (const auto& info : _pendingCreationInfo) {
            const auto& resources = *info.reflection;
            for (const auto& [set, bindings] : resources.descriptorSets) {
                for (const auto& binding : bindings) {
                    auto& mergedBinding = mergedBindings[set][binding.binding];
                    mergedBinding.binding = binding.binding;
                    mergedBinding.descriptorType = binding.descriptorType;
                    mergedBinding.descriptorCount = binding.descriptorCount;
                    mergedBinding.stageFlags |= binding.stageFlags;
                    mergedBinding.pImmutableSamplers = binding.pImmutableSamplers;
                }
            }
        }

        uint32_t maxSetIndex = 0;
        if (!mergedBindings.empty()) {
            maxSetIndex = mergedBindings.rbegin()->first;
        }

        if (_config.layoutOverride) {
            for (uint32_t probeSet = maxSetIndex + 1; probeSet < 4; ++probeSet) {
                if (_config.layoutOverride(probeSet) != VK_NULL_HANDLE) {
                    maxSetIndex = probeSet;
                }
            }
        }

        _bindLayouts.resize(maxSetIndex + 1, VK_NULL_HANDLE);

        for (uint32_t set = 0; set <= maxSetIndex; ++set) 
        {
            if (_config.layoutOverride) {
                VkDescriptorSetLayout overridenLayout = _config.layoutOverride(set);
                if (overridenLayout != VK_NULL_HANDLE) {
                    _bindLayouts[set] = overridenLayout;
                    continue;
                }
            }

            if (mergedBindings.contains(set)) {
                std::vector<VkDescriptorSetLayoutBinding> bindingsVec;
                std::vector<VkDescriptorBindingFlags> bindingFlags;

                for (const auto& [bIdx, bind] : mergedBindings[set]) {
                    bindingsVec.push_back(bind);

                    if (bind.descriptorCount > 1) {
                        bindingFlags.push_back(VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT | VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT);
                    }
                    else {
                        bindingFlags.push_back(0);
                    }
                }

                VkDescriptorSetLayoutBindingFlagsCreateInfo flagsInfo{ VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_BINDING_FLAGS_CREATE_INFO };
                flagsInfo.bindingCount = static_cast<uint32_t>(bindingFlags.size());
                flagsInfo.pBindingFlags = bindingFlags.data();

                VkDescriptorSetLayoutCreateInfo layoutInfo = { VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO };
                layoutInfo.bindingCount = static_cast<uint32_t>(bindingsVec.size());
                layoutInfo.pBindings = bindingsVec.data();
                layoutInfo.pNext = &flagsInfo;
                layoutInfo.flags = 0;

                if (_config.useDescriptorBuffers) {
                    layoutInfo.flags |= VK_DESCRIPTOR_SET_LAYOUT_CREATE_DESCRIPTOR_BUFFER_BIT_EXT;
                }
                else {
                    layoutInfo.flags |= VK_DESCRIPTOR_SET_LAYOUT_CREATE_UPDATE_AFTER_BIND_POOL_BIT;
                }

                if (set == 2) {
                    layoutInfo.flags |= VK_DESCRIPTOR_SET_LAYOUT_CREATE_PUSH_DESCRIPTOR_BIT_KHR;
                }

                VkDescriptorSetLayout setLayout;
                SYN_VK_ASSERT_MSG(vkCreateDescriptorSetLayout(device->Handle(), &layoutInfo, nullptr, &setLayout), "Failed to create Descriptor Set Layout");

                _createdLayouts.push_back(setLayout);
                _bindLayouts[set] = setLayout;
            }
            else {
                _bindLayouts[set] = DescriptorUtils::GetEmptyDescriptorSetLayout(_config.useDescriptorBuffers);
            }
        }

        auto physicalDevice = ServiceLocator::Get<Vk::Context>()->GetPhysicalDevice();
        uint32_t maxPushConstantSize = physicalDevice->GetProperties().limits.maxPushConstantsSize;

        VkPushConstantRange universalPushConstant{};
        universalPushConstant.stageFlags = VK_SHADER_STAGE_ALL;
        universalPushConstant.offset = 0;
        universalPushConstant.size = maxPushConstantSize;

        VkPipelineLayoutCreateInfo pipelineLayoutInfo{ VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO };
        if (!_bindLayouts.empty()) {
            pipelineLayoutInfo.setLayoutCount = static_cast<uint32_t>(_bindLayouts.size());
            pipelineLayoutInfo.pSetLayouts = _bindLayouts.data();
        }

        pipelineLayoutInfo.pushConstantRangeCount = 1;
        pipelineLayoutInfo.pPushConstantRanges = &universalPushConstant;

        SYN_VK_ASSERT_MSG(vkCreatePipelineLayout(device->Handle(), &pipelineLayoutInfo, nullptr, &_pipelineLayout), "Failed to create merged Pipeline Layout");

        _shaderObjects.resize(_pendingCreationInfo.size());
        _stages.resize(_pendingCreationInfo.size());

        bool hasTaskShader = false;
        for (const auto& info : _pendingCreationInfo) {
            if (info.stage == VK_SHADER_STAGE_TASK_BIT_EXT) {
                hasTaskShader = true;
                break;
            }
        }

        for (size_t i = 0; i < _pendingCreationInfo.size(); ++i) {
            const auto& info = _pendingCreationInfo[i];
            _stages[i] = info.stage;
            const auto& spirv = *info.spirv;

            VkShaderStageFlags nextStages = 0;
            switch (info.stage) {
            case VK_SHADER_STAGE_VERTEX_BIT:
                nextStages = VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT | VK_SHADER_STAGE_GEOMETRY_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
                break;
            case VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT:
                nextStages = VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT;
                break;
            case VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT:
                nextStages = VK_SHADER_STAGE_GEOMETRY_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
                break;
            case VK_SHADER_STAGE_GEOMETRY_BIT:
                nextStages = VK_SHADER_STAGE_FRAGMENT_BIT;
                break;
            case VK_SHADER_STAGE_FRAGMENT_BIT:
                nextStages = 0;
                break;
            case VK_SHADER_STAGE_TASK_BIT_EXT:
                nextStages = VK_SHADER_STAGE_MESH_BIT_EXT;
                break;
            case VK_SHADER_STAGE_MESH_BIT_EXT:
                nextStages = VK_SHADER_STAGE_FRAGMENT_BIT;
                break;
            default:
                nextStages = 0;
                break;
            }

            VkShaderCreateInfoEXT createInfo = { VK_STRUCTURE_TYPE_SHADER_CREATE_INFO_EXT };
            createInfo.nextStage = nextStages;
            createInfo.stage = info.stage;
            createInfo.codeType = VK_SHADER_CODE_TYPE_SPIRV_EXT;
            createInfo.pCode = spirv.data();
            createInfo.codeSize = spirv.size() * sizeof(uint32_t);
            createInfo.pName = "main";

            if (info.stage == VK_SHADER_STAGE_MESH_BIT_EXT && !hasTaskShader)
                createInfo.flags = VK_SHADER_CREATE_NO_TASK_SHADER_BIT_EXT;
            else
                createInfo.flags = 0;

            createInfo.pushConstantRangeCount = 1;
            createInfo.pPushConstantRanges = &universalPushConstant;

            if (!_bindLayouts.empty()) {
                createInfo.pSetLayouts = _bindLayouts.data();
                createInfo.setLayoutCount = static_cast<uint32_t>(_bindLayouts.size());
            }

            SYN_VK_ASSERT_MSG(vkCreateShadersEXT(device->Handle(), 1, &createInfo, nullptr, &_shaderObjects[i]), "Failed to create Shader Object EXT from Program!");

            if (info.outHandle) {
                *info.outHandle = _shaderObjects[i];
            }
        }

        _pendingCreationInfo.clear();
    }
}