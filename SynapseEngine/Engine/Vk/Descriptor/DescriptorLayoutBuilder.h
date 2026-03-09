#pragma once
#include "../VkCommon.h"
#include <vector>
#include <cstdint>

namespace Syn::Vk {

    enum class DescriptorLayoutType {
        Standard,
        DescriptorBuffer,
        PushDescriptor
    };

    class SYN_API DescriptorLayoutBuilder {
    public:
        DescriptorLayoutBuilder() = default;
        ~DescriptorLayoutBuilder() = default;

        DescriptorLayoutBuilder& AddBinding(
            uint32_t binding,
            VkDescriptorType type,
            VkShaderStageFlags stageFlags,
            uint32_t count = 1,
            VkDescriptorBindingFlags bindingFlags = 0
        );

        DescriptorLayoutBuilder& AddBindlessBinding(
            uint32_t binding,
            VkDescriptorType type,
            VkShaderStageFlags stageFlags,
            uint32_t count
        );

        VkDescriptorSetLayout Build(DescriptorLayoutType type = DescriptorLayoutType::Standard);
    private:
        std::vector<VkDescriptorSetLayoutBinding> _bindings;
        std::vector<VkDescriptorBindingFlags> _bindingFlags;
    };
}