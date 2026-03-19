#pragma once
#include "../VkCommon.h"
#include "Shader.h"

namespace Syn::Vk {

    using DescriptorLayoutOverride = std::function<VkDescriptorSetLayout(uint32_t setIndex)>;

    struct ShaderProgramConfig {
        DescriptorLayoutOverride layoutOverride = nullptr;
        bool useDescriptorBuffers = false;
    };

    class SYN_API ShaderProgram {
    public:
        ShaderProgram(std::span<const Shader* const> shaders, const ShaderProgramConfig& config = {});
        ~ShaderProgram();

        ShaderProgram(const ShaderProgram&) = delete;
        ShaderProgram& operator=(const ShaderProgram&) = delete;

        void Bind(VkCommandBuffer cmd) const;

        VkPipelineLayout GetLayout() const { return _pipelineLayout; }
        const std::vector<const Shader*>& GetShaders() const { return _shaders; }
    private:
        void CreatePipelineLayoutAndShaders();
    private:
        VkPipelineLayout _pipelineLayout = VK_NULL_HANDLE;
        std::vector<const Shader*> _shaders;

        ShaderProgramConfig _config;
        std::vector<VkShaderEXT> _shaderObjects;
        std::vector<VkDescriptorSetLayout> _createdLayouts;
        std::vector<VkDescriptorSetLayout> _bindLayouts;
    };
}