#pragma once
#include "../VkCommon.h"
#include <shaderc/shaderc.hpp>

namespace Syn::Vk {
    class SYN_API ShaderCompiler {
    public:
        static std::vector<uint32_t> Compile(const std::string& filepath, VkShaderStageFlagBits stage);
    private:
        static std::string LoadFile(const std::string& filepath);
        static shaderc_shader_kind MapStageToKind(VkShaderStageFlagBits stage);
    };
}