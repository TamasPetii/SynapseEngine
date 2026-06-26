#pragma once
#include <volk.h>

namespace Syn::Vk {

    template<typename T>
    struct PushConstant {
        T data{};

        T* operator->() { return &data; }
        const T* operator->() const { return &data; }

        T& operator*() { return data; }
        const T& operator*() const { return data; }

        void Push(VkCommandBuffer cmd, VkPipelineLayout layout, VkShaderStageFlags stages = VK_SHADER_STAGE_ALL, uint32_t offset = 0) const {
            vkCmdPushConstants(cmd, layout, stages, offset, sizeof(T), &data);
        }
    };

}