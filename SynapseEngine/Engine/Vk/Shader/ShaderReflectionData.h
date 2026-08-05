#pragma once
#include "Engine/SynApi.h"
#include "Engine/Vk/VkCommon.h"
#include <vector>
#include <cstdint>

namespace Syn
{
    struct SYN_API ShaderResourceVariable
    {
        std::string name;
        uint32_t offset;
        uint32_t size;
        uint32_t type;
    };

    struct SYN_API ShaderResources
    {
        std::vector<VkDescriptorSetLayoutBinding> bindings;
        std::map<uint32_t, std::vector<VkDescriptorSetLayoutBinding>> descriptorSets;

        std::vector<VkPushConstantRange> pushConstants;
        std::unordered_map<std::string, ShaderResourceVariable> pushConstantMembers;
    };

}

