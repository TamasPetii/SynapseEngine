#include "Shader.h"
#include "ShaderCompiler.h"

namespace Syn::Vk {
    Shader::Shader(const std::string& filepath, VkShaderStageFlagBits stage, std::span<const std::string> defines)
        : _stage(stage)
    {
        _spirv = ShaderCompiler::Compile(filepath, stage, defines);
        _resources = ShaderReflector::Reflect(_spirv, stage, filepath);
    }
}