#pragma once
#include "../VkCommon.h"
#include "ShaderReflector.h"

namespace Syn::Vk {
    class SYN_API Shader {
    public:
        Shader(const std::string& filepath, VkShaderStageFlagBits stage);
        ~Shader();

        Shader(const Shader&) = delete;
        Shader& operator=(const Shader&) = delete;

        VkShaderEXT Handle() const { return _handle; }
        VkShaderStageFlagBits GetStage() const { return _stage; }
        const ShaderResources& GetResources() const { return _resources; }
        const std::vector<VkDescriptorSetLayout>& GetDescriptorSetLayouts() const { return _descriptorSetLayouts; }
    private:
        VkShaderEXT _handle = VK_NULL_HANDLE;
        VkShaderStageFlagBits _stage;
        ShaderResources _resources;
        std::vector<VkDescriptorSetLayout> _descriptorSetLayouts;
    };
}