#include "Shader.h"
#include "ShaderCompiler.h"
#include "Engine/SynMacro.h"
#include "Engine/ServiceLocator.h"
#include "Engine/Vk/Context.h"
#include "ShaderReflector.h"

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

        _descriptorSetLayouts.resize(maxSetIndex, VK_NULL_HANDLE);

        for (const auto& [setIndex, bindings] : _resources.descriptorSets) {
            VkDescriptorSetLayoutCreateInfo layoutInfo = { VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO };
            layoutInfo.bindingCount = static_cast<uint32_t>(bindings.size());
            layoutInfo.pBindings = bindings.data();

			//Set 0 -> Global Descriptor Buffer Extension
            if (setIndex == 0) {
                layoutInfo.flags = VK_DESCRIPTOR_SET_LAYOUT_CREATE_DESCRIPTOR_BUFFER_BIT_EXT;
            }
			//Set != 0 -> Push Descriptor Extension
            else {
                layoutInfo.flags = VK_DESCRIPTOR_SET_LAYOUT_CREATE_PUSH_DESCRIPTOR_BIT_KHR;
            }

            VkDescriptorSetLayout setLayout;
            SYN_VK_ASSERT_MSG(vkCreateDescriptorSetLayout(device->Handle(), &layoutInfo, nullptr, &setLayout), "Failed to create descriptor set layout from reflection!");

            _descriptorSetLayouts[setIndex] = setLayout;
        }

        // Shader Object
        VkShaderCreateInfoEXT createInfo = { VK_STRUCTURE_TYPE_SHADER_CREATE_INFO_EXT };
        createInfo.stage = stage;
        createInfo.codeType = VK_SHADER_CODE_TYPE_SPIRV_EXT;
        createInfo.pCode = spirv.data();
        createInfo.codeSize = spirv.size() * sizeof(uint32_t);
        createInfo.pName = "main";

        if (!_resources.pushConstants.empty()) {
            createInfo.pPushConstantRanges = _resources.pushConstants.data();
            createInfo.pushConstantRangeCount = static_cast<uint32_t>(_resources.pushConstants.size());
        }

        if (!_descriptorSetLayouts.empty()) {
            createInfo.pSetLayouts = _descriptorSetLayouts.data();
            createInfo.setLayoutCount = static_cast<uint32_t>(_descriptorSetLayouts.size());
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