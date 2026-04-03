#pragma once
#include "../VkCommon.h"
#include <spirv_reflect.h>

namespace Syn::Vk {

    struct SYN_API ShaderResourceVariable {
        std::string name;
        uint32_t offset;
        uint32_t size;
        uint32_t type;
    };

    struct SYN_API ShaderResources {
        std::vector<VkDescriptorSetLayoutBinding> bindings;
        std::map<uint32_t, std::vector<VkDescriptorSetLayoutBinding>> descriptorSets;

        std::vector<VkPushConstantRange> pushConstants;
        std::unordered_map<std::string, ShaderResourceVariable> pushConstantMembers;
    };

    class SYN_API ShaderReflector {
    public:
        static ShaderResources Reflect(const std::vector<uint32_t>& spirvCode, VkShaderStageFlagBits stage, const std::string& shaderName);
    private:
        static void LogReflectionInfo(const std::string& shaderName,
            const std::span<SpvReflectDescriptorBinding*> bindings,
            const std::span<SpvReflectBlockVariable*> pushBlocks);
    };
}