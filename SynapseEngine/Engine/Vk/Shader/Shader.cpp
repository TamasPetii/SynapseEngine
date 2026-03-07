#include "Shader.h"
#include "ShaderCompiler.h"
#include "Engine/ServiceLocator.h"
#include "Engine/Vk/Context.h"
#include "Engine/Vk/Descriptor/DescriptorUtils.h"

namespace Syn::Vk {
    Shader::Shader(const std::string& filepath, VkShaderStageFlagBits stage)
        : _stage(stage)
    {
        auto device = ServiceLocator::GetVkContext()->GetDevice();

        std::vector<uint32_t> spirv = ShaderCompiler::Compile(filepath, stage);
        _resources = ShaderReflector::Reflect(spirv, stage, filepath);

        uint32_t maxSetIndex = 0;
        if (!_resources.descriptorSets.empty()) {
            maxSetIndex = _resources.descriptorSets.rbegin()->first + 1;
        }

		//Filling out empty sets if any
        _descriptorSetLayouts.resize(maxSetIndex, VK_NULL_HANDLE);

        for (const auto& [setIndex, bindings] : _resources.descriptorSets) {
            VkDescriptorSetLayoutCreateInfo layoutInfo = { VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO };
            layoutInfo.bindingCount = static_cast<uint32_t>(bindings.size());
            layoutInfo.pBindings = bindings.data();
            layoutInfo.flags = VK_DESCRIPTOR_SET_LAYOUT_CREATE_DESCRIPTOR_BUFFER_BIT_EXT;

            /* NOT WORKING, CANNOT MIX BUFFER WITH PUSH DESCRIPTOR
            * 
			//Set 0 -> Global Descriptor Buffer Extension
            if (setIndex == 0) {
                layoutInfo.flags = VK_DESCRIPTOR_SET_LAYOUT_CREATE_DESCRIPTOR_BUFFER_BIT_EXT;
            }
			//Set != 0 -> Push Descriptor Extension
            else {
                layoutInfo.flags = VK_DESCRIPTOR_SET_LAYOUT_CREATE_PUSH_DESCRIPTOR_BIT_KHR;
            }
            */

            VkDescriptorSetLayout setLayout;
            SYN_VK_ASSERT_MSG(vkCreateDescriptorSetLayout(device->Handle(), &layoutInfo, nullptr, &setLayout), "Failed to create descriptor set layout from reflection!");

            _descriptorSetLayouts[setIndex] = setLayout;
        }

        //Vulkan gives validation error to empty VK_NULL_HANDLE layouts -> Need to fill them up
        std::vector<VkDescriptorSetLayout> creationLayouts = _descriptorSetLayouts;
        if (!creationLayouts.empty()) {
            VkDescriptorSetLayout emptyLayout = DescriptorUtils::GetEmptyDescriptorSetLayout();

            for (auto& layout : creationLayouts) {
                if (layout == VK_NULL_HANDLE) {
                    layout = emptyLayout;
                }
            }
        }

        VkShaderStageFlags nextStages = 0;
        switch (stage) {
        case VK_SHADER_STAGE_VERTEX_BIT:
            nextStages = VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT |
                VK_SHADER_STAGE_GEOMETRY_BIT |
                VK_SHADER_STAGE_FRAGMENT_BIT;
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

        // Shader Object
        VkShaderCreateInfoEXT createInfo = { VK_STRUCTURE_TYPE_SHADER_CREATE_INFO_EXT };
        createInfo.nextStage = nextStages;
        createInfo.stage = stage;
        createInfo.codeType = VK_SHADER_CODE_TYPE_SPIRV_EXT;
        createInfo.pCode = spirv.data();
        createInfo.codeSize = spirv.size() * sizeof(uint32_t);
        createInfo.pName = "main";

        if (stage == VK_SHADER_STAGE_MESH_BIT_EXT) {
            createInfo.flags = VK_SHADER_CREATE_NO_TASK_SHADER_BIT_EXT;
        }
        else {
            createInfo.flags = 0;
        }

        /*
        if (!_resources.pushConstants.empty()) {
            createInfo.pPushConstantRanges = _resources.pushConstants.data();
            createInfo.pushConstantRangeCount = static_cast<uint32_t>(_resources.pushConstants.size());
        }
        */

        auto physicalDevice = ServiceLocator::GetVkContext()->GetPhysicalDevice();
        uint32_t maxPushConstantSize = physicalDevice->GetProperties().limits.maxPushConstantsSize;

        VkPushConstantRange universalPushConstant{};
        universalPushConstant.stageFlags = VK_SHADER_STAGE_ALL_GRAPHICS;
        universalPushConstant.offset = 0;
        universalPushConstant.size = maxPushConstantSize;

        createInfo.pushConstantRangeCount = 1;
        createInfo.pPushConstantRanges = &universalPushConstant;

        if (!creationLayouts.empty()) {
            createInfo.pSetLayouts = creationLayouts.data();
            createInfo.setLayoutCount = static_cast<uint32_t>(creationLayouts.size());
        }
        
        SYN_VK_ASSERT_MSG(vkCreateShadersEXT(device->Handle(), 1, &createInfo, nullptr, &_handle), ("Failed to create Shader Object EXT: " + filepath).c_str());
    }

    Shader::~Shader() {
        auto device = ServiceLocator::GetVkContext()->GetDevice();

        if (_handle != VK_NULL_HANDLE) {
            vkDestroyShaderEXT(device->Handle(), _handle, nullptr);
        }

        for (auto layout : _descriptorSetLayouts) {
            if (layout != VK_NULL_HANDLE) {
                vkDestroyDescriptorSetLayout(device->Handle(), layout, nullptr);
            }
        }
    }
}