#pragma once
#include "../VkCommon.h"
#include <span>
#include "ShaderReflector.h"

namespace Syn::Vk {
    class SYN_API Shader {
    public:
        Shader(const std::string& filepath, VkShaderStageFlagBits stage);
        ~Shader() = default;

        Shader(const Shader&) = delete;
        Shader& operator=(const Shader&) = delete;

        VkShaderStageFlagBits GetStage() const { return _stage; }
        const ShaderResources& GetResources() const { return _resources; }
        const std::span<const uint32_t> GetSpirv() const { return _spirv; }
    private:
        VkShaderStageFlagBits _stage;
        ShaderResources _resources;
        std::vector<uint32_t> _spirv;
    };
}