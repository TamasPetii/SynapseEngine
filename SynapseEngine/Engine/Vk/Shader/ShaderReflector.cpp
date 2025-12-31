#include "ShaderReflector.h"
#include "Engine/SynMacro.h"
#include "Engine/Logger/Logger.h"
#include <print>

namespace Syn::Vk {
    ShaderResources ShaderReflector::Reflect(const std::vector<uint32_t>& spirvCode, VkShaderStageFlagBits stage, const std::string& shaderName) {
        ShaderResources resources;

        spv_reflect::ShaderModule module(spirvCode);

        SYN_ASSERT(module.GetResult() == SPV_REFLECT_RESULT_SUCCESS, "Failed to reflect shader SPIR-V!");

        // Descriptor Bindings
        uint32_t count = 0;
        module.EnumerateDescriptorBindings(&count, nullptr);
        std::vector<SpvReflectDescriptorBinding*> bindings(count);
        module.EnumerateDescriptorBindings(&count, bindings.data());

        for (const auto* binding : bindings) {
            VkDescriptorSetLayoutBinding layoutBinding{};
            layoutBinding.binding = binding->binding;
            layoutBinding.descriptorType = static_cast<VkDescriptorType>(binding->descriptor_type);

            // Arrays
            layoutBinding.descriptorCount = 1;
            for (uint32_t i = 0; i < binding->array.dims_count; ++i) {
                layoutBinding.descriptorCount *= binding->array.dims[i];
            }

            layoutBinding.stageFlags = stage;
            layoutBinding.pImmutableSamplers = nullptr;

            resources.bindings.push_back(layoutBinding);
            resources.descriptorSets[binding->set].push_back(layoutBinding);
        }

        // Push Constants
        uint32_t pushCount = 0;
        module.EnumeratePushConstantBlocks(&pushCount, nullptr);
        std::vector<SpvReflectBlockVariable*> pushBlocks(pushCount);
        module.EnumeratePushConstantBlocks(&pushCount, pushBlocks.data());

        for (const auto* block : pushBlocks) {
            VkPushConstantRange range{};
            range.offset = block->offset;
            range.size = block->size;
            range.stageFlags = stage;

            resources.pushConstants.push_back(range);

            if (block->member_count > 0) {
                for (uint32_t i = 0; i < block->member_count; ++i) {
                    const auto& member = block->members[i];

                    ShaderResourceVariable varInfo;
                    varInfo.name = member.name ? member.name : "";
                    varInfo.offset = member.absolute_offset;
                    varInfo.size = member.size;
                    varInfo.type = member.type_description->op;

                    if (!varInfo.name.empty()) {
                        resources.pushConstantMembers[varInfo.name] = varInfo;
                    }
                }
            }
        }

		LogReflectionInfo(shaderName, bindings, pushBlocks);

        return resources;
    }

    void ShaderReflector::LogReflectionInfo(
        const std::string& shaderName,
        const std::span<SpvReflectDescriptorBinding*> bindings,
        const std::span<SpvReflectBlockVariable*> pushBlocks)
    {

        if constexpr (!Syn::EnableLogging) {
            return;
        }

        std::stringstream ss;
        ss << "\n======================================================\n";
        ss << " [SHADER REFLECTION] " << shaderName << "\n";
        ss << "======================================================\n";

        if (!bindings.empty()) {
            ss << "--- Descriptor Sets ---\n";
            for (const auto* binding : bindings) {
                uint32_t descCount = 1;
                for (uint32_t i = 0; i < binding->array.dims_count; ++i) descCount *= binding->array.dims[i];

                ss << " Set " << binding->set
                    << " | Binding " << binding->binding
                    << " | Count " << descCount
                    << " | Name: " << (binding->name ? binding->name : "UNNAMED")
                    << "\n";
            }
        }

        if (!pushBlocks.empty()) {
            ss << "--- Push Constants ---\n";
            for (const auto* block : pushBlocks) {
                ss << " Block: " << (block->name ? block->name : "UNNAMED")
                    << " | Offset: " << block->offset
                    << " | Size: " << block->size << "\n";

                for (uint32_t i = 0; i < block->member_count; ++i) {
                    const auto& member = block->members[i];
                    ss << "    -> Member: " << (member.name ? member.name : "UNNAMED")
                        << " | Offset: " << member.absolute_offset
                        << " | Size: " << member.size << "\n";
                }
            }
        }
        ss << "------------------------------------------------------";

        Info("{}", ss.str());
    }
}