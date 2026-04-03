#include "DescriptorLayoutBuilder.h"
#include "Engine/ServiceLocator.h"
#include "Engine/Vk/Context.h"
#include "Engine/Vk/Core/Device.h"

namespace Syn::Vk {

    DescriptorLayoutBuilder& DescriptorLayoutBuilder::AddBindlessBinding(uint32_t binding, VkDescriptorType type, VkShaderStageFlags stageFlags, uint32_t count)
    {
        VkDescriptorBindingFlags bindlessFlags = VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT;

        return AddBinding(binding, type, stageFlags, count, bindlessFlags);
    }

    DescriptorLayoutBuilder& DescriptorLayoutBuilder::AddBinding(uint32_t binding, VkDescriptorType type, VkShaderStageFlags stageFlags, uint32_t count, VkDescriptorBindingFlags bindingFlags)
    {
        VkDescriptorSetLayoutBinding layoutBinding{};
        layoutBinding.binding = binding;
        layoutBinding.descriptorType = type;
        layoutBinding.descriptorCount = count;
        layoutBinding.stageFlags = stageFlags;

        _bindings.push_back(layoutBinding);
        _bindingFlags.push_back(bindingFlags);
        return *this;
    }

    VkDescriptorSetLayout DescriptorLayoutBuilder::Build(DescriptorLayoutType type)
    {
        auto device = ServiceLocator::GetVkContext()->GetDevice();

        VkDescriptorSetLayoutCreateInfo layoutInfo{ VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO };
        layoutInfo.bindingCount = static_cast<uint32_t>(_bindings.size());
        layoutInfo.pBindings = _bindings.data();
        layoutInfo.flags = 0;

        if (type == DescriptorLayoutType::PushDescriptor) {
            layoutInfo.flags |= VK_DESCRIPTOR_SET_LAYOUT_CREATE_PUSH_DESCRIPTOR_BIT_KHR;
            layoutInfo.flags |= VK_DESCRIPTOR_SET_LAYOUT_CREATE_DESCRIPTOR_BUFFER_BIT_EXT;
        }
        else if (type == DescriptorLayoutType::DescriptorBuffer) {
            layoutInfo.flags |= VK_DESCRIPTOR_SET_LAYOUT_CREATE_DESCRIPTOR_BUFFER_BIT_EXT;
        }

        VkDescriptorSetLayoutBindingFlagsCreateInfo flagsInfo{ VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_BINDING_FLAGS_CREATE_INFO };
        flagsInfo.bindingCount = static_cast<uint32_t>(_bindingFlags.size());
        flagsInfo.pBindingFlags = _bindingFlags.data();

        bool hasFlags = false;
        for (auto f : _bindingFlags) {
            if (f != 0) hasFlags = true;
        }

        if (hasFlags) 
        {
            layoutInfo.pNext = &flagsInfo;

            if (type == DescriptorLayoutType::Standard) {
                layoutInfo.flags |= VK_DESCRIPTOR_SET_LAYOUT_CREATE_UPDATE_AFTER_BIND_POOL_BIT;
            }
        }

        VkDescriptorSetLayout layout = VK_NULL_HANDLE;
        SYN_VK_ASSERT_MSG(vkCreateDescriptorSetLayout(device->Handle(), &layoutInfo, nullptr, &layout), "Failed to build Descriptor Set Layout");

        _bindings.clear();
        _bindingFlags.clear();
        return layout;
    }
}