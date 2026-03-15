#include "Shader.h"
#include "ShaderCompiler.h"

namespace Syn::Vk {
    Shader::Shader(const std::string& filepath, VkShaderStageFlagBits stage)
        : _stage(stage)
    {
        _spirv = ShaderCompiler::Compile(filepath, stage);
        _resources = ShaderReflector::Reflect(_spirv, stage, filepath);
    }
}