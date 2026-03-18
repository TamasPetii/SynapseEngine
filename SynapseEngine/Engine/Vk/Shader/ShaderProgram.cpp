#include "ShaderProgram.h"
#include "Engine/ServiceLocator.h"
#include "Engine/Vk/Context.h"
#include "Engine/Vk/Descriptor/DescriptorUtils.h"
#include "Engine/Logger/SynLog.h"

namespace Syn::Vk {

    ShaderProgram::ShaderProgram(std::span<const Shader* const> shaders) {
        _shaders.assign(shaders.begin(), shaders.end());
        CreatePipelineLayoutAndShaders();
    }

    ShaderProgram::~ShaderProgram() {
        auto device = ServiceLocator::GetVkContext()->GetDevice();

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
        static const std::vector<VkShaderStageFlagBits> allStages = {
            VK_SHADER_STAGE_VERTEX_BIT,
            VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT,
            VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT,
            VK_SHADER_STAGE_GEOMETRY_BIT,
            VK_SHADER_STAGE_FRAGMENT_BIT,
            VK_SHADER_STAGE_TASK_BIT_EXT,
            VK_SHADER_STAGE_MESH_BIT_EXT,
            VK_SHADER_STAGE_COMPUTE_BIT
        };

        std::vector<VkShaderEXT> handlesToBind(allStages.size(), VK_NULL_HANDLE);
        for (size_t i = 0; i < _shaders.size(); ++i) {
            VkShaderStageFlagBits currentStage = _shaders[i]->GetStage();

            for (size_t j = 0; j < allStages.size(); ++j) {
                if (allStages[j] == currentStage) {
                    handlesToBind[j] = _shaderObjects[i];
                    break;
                }
            }
        }

        vkCmdBindShadersEXT(
            cmd,
            static_cast<uint32_t>(allStages.size()),
            allStages.data(),
            handlesToBind.data()
        );
    }

    void ShaderProgram::CreatePipelineLayoutAndShaders() {
        auto device = ServiceLocator::GetVkContext()->GetDevice();

        std::map<uint32_t, std::map<uint32_t, VkDescriptorSetLayoutBinding>> mergedBindings;

        for (const auto* shader : _shaders) {
            const auto& resources = shader->GetResources();
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

        _bindLayouts.resize(maxSetIndex + 1, VK_NULL_HANDLE);
        for (uint32_t set = 0; set <= maxSetIndex; ++set) {
            if (mergedBindings.contains(set)) {
                std::vector<VkDescriptorSetLayoutBinding> bindingsVec;
                for (const auto& [bIdx, bind] : mergedBindings[set]) {
                    bindingsVec.push_back(bind);
                }

                VkDescriptorSetLayoutCreateInfo layoutInfo = { VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO };
                layoutInfo.bindingCount = static_cast<uint32_t>(bindingsVec.size());
                layoutInfo.pBindings = bindingsVec.data();
                layoutInfo.flags = VK_DESCRIPTOR_SET_LAYOUT_CREATE_DESCRIPTOR_BUFFER_BIT_EXT;

                if (set == 2) {
                    layoutInfo.flags |= VK_DESCRIPTOR_SET_LAYOUT_CREATE_PUSH_DESCRIPTOR_BIT_KHR;
                }

                VkDescriptorSetLayout setLayout;
                SYN_VK_ASSERT_MSG(vkCreateDescriptorSetLayout(device->Handle(), &layoutInfo, nullptr, &setLayout), "Failed to create merged descriptor set layout!");

                _createdLayouts.push_back(setLayout);
                _bindLayouts[set] = setLayout;
            }
            else {
                _bindLayouts[set] = DescriptorUtils::GetEmptyDescriptorSetLayout();
            }
        }

        auto physicalDevice = ServiceLocator::GetVkContext()->GetPhysicalDevice();
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

        _shaderObjects.resize(_shaders.size());

        bool hasTaskShader = false;
        for (const auto* shader : _shaders) {
            if (shader->GetStage() == VK_SHADER_STAGE_TASK_BIT_EXT) {
                hasTaskShader = true;
                break;
            }
        }

        for (size_t i = 0; i < _shaders.size(); ++i) {
            const auto* shader = _shaders[i];
            auto stage = shader->GetStage();
            const auto& spirv = shader->GetSpirv();

            VkShaderStageFlags nextStages = 0;
            switch (stage) {
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
            createInfo.stage = stage;
            createInfo.codeType = VK_SHADER_CODE_TYPE_SPIRV_EXT;
            createInfo.pCode = spirv.data();
            createInfo.codeSize = spirv.size() * sizeof(uint32_t);
            createInfo.pName = "main";

            if (stage == VK_SHADER_STAGE_MESH_BIT_EXT && !hasTaskShader)
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
        }
    }
}