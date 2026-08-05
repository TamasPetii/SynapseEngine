#pragma once
#include "IShaderReflector.h"
#include <span>
#include <spirv-reflect/spirv_reflect.h>

namespace Syn {
    class SYN_API SpirvShaderReflector : public IShaderReflector {
    public:
        SpirvShaderReflector() = default;
        ~SpirvShaderReflector() override = default;

        ShaderResources Reflect(const std::vector<uint32_t>& bytecode, VkShaderStageFlagBits stage, const std::string& shaderName) override;
    private:
        void LogReflectionInfo(const std::string& shaderName, const std::span<SpvReflectDescriptorBinding*> bindings, const std::span<SpvReflectBlockVariable*> pushBlocks);
    };
}