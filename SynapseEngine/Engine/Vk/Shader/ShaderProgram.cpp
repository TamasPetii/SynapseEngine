#include "ShaderProgram.h"
#include "Engine/Vk/Rendering/RenderUtils.h"
#include "Engine/ServiceLocator.h"
#include "Engine/Vk/Context.h"
#include "Engine/Vk/Descriptor/DescriptorUtils.h"

namespace Syn::Vk {

    ShaderProgram::ShaderProgram(std::span<const Shader* const> shaders) {
        _shaders.assign(shaders.begin(), shaders.end());
        CreatePipelineLayout();
    }

    ShaderProgram::~ShaderProgram() {
        if (_pipelineLayout != VK_NULL_HANDLE) {
            auto device = ServiceLocator::GetVkContext()->GetDevice();
            vkDestroyPipelineLayout(device->Handle(), _pipelineLayout, nullptr);
            _pipelineLayout = VK_NULL_HANDLE;
        }
    }

    void ShaderProgram::Bind(VkCommandBuffer cmd) const {
        RenderUtils::BindShaders(cmd, _shaders);
    }

    void ShaderProgram::CreatePipelineLayout() {
        auto device = ServiceLocator::GetVkContext()->GetDevice();

        std::map<uint32_t, VkDescriptorSetLayout> setLayoutsMap;

		// Collect unique descriptor set layouts from all shaders
        for (const auto* shader : _shaders) {
            const auto& shaderLayouts = shader->GetDescriptorSetLayouts();

            for (uint32_t i = 0; i < shaderLayouts.size(); ++i) {
                if (shaderLayouts[i] != VK_NULL_HANDLE) {
                    if (setLayoutsMap.find(i) == setLayoutsMap.end()) {
                        setLayoutsMap[i] = shaderLayouts[i];
                    }
                }
            }
        }

        std::vector<VkDescriptorSetLayout> finalSetLayouts;
        if (!setLayoutsMap.empty()) {
            uint32_t maxSetIndex = setLayoutsMap.rbegin()->first;
            finalSetLayouts.resize(maxSetIndex + 1, VK_NULL_HANDLE);

            for (const auto& [index, layout] : setLayoutsMap) {
                finalSetLayouts[index] = layout;
            }

            for (auto& layout : finalSetLayouts) {
                if (layout == VK_NULL_HANDLE) {
                    layout = DescriptorUtils::GetEmptyDescriptorSetLayout();
                }
            }
        }

		// Merge push constant ranges at same offset and size, combining their stage flags
        std::map<std::pair<uint32_t, uint32_t>, VkShaderStageFlags> rangeMerger;
        for (const auto* shader : _shaders) {
            const auto& resources = shader->GetResources();
            for (const auto& pc : resources.pushConstants) {
                std::pair<uint32_t, uint32_t> key = { pc.offset, pc.size };
                rangeMerger[key] |= shader->GetStage();
            }
        }

        std::vector<VkPushConstantRange> finalPushConstants;
        finalPushConstants.reserve(rangeMerger.size());

        for (const auto& [key, stages] : rangeMerger) {
            VkPushConstantRange range{};
            range.offset = key.first;
            range.size = key.second;
            range.stageFlags = stages;
            finalPushConstants.push_back(range);
        }

        VkPipelineLayoutCreateInfo pipelineLayoutInfo{ VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO };

        if (!finalSetLayouts.empty()) {
            pipelineLayoutInfo.setLayoutCount = static_cast<uint32_t>(finalSetLayouts.size());
            pipelineLayoutInfo.pSetLayouts = finalSetLayouts.data();
        }

        if (!finalPushConstants.empty()) {
            pipelineLayoutInfo.pushConstantRangeCount = static_cast<uint32_t>(finalPushConstants.size());
            pipelineLayoutInfo.pPushConstantRanges = finalPushConstants.data();
        }

        SYN_VK_ASSERT_MSG(vkCreatePipelineLayout(device->Handle(), &pipelineLayoutInfo, nullptr, &_pipelineLayout), "Failed to create merged Pipeline Layout");
    }
}