#pragma once
#include "../IShaderCompiler.h"
#include <shaderc/shaderc.hpp>

namespace Syn {
    class SYN_API ShadercShaderCompiler : public IShaderCompiler {
    public:
        ShadercShaderCompiler() = default;
        ~ShadercShaderCompiler() override = default;

        std::vector<uint32_t> Compile(const CookedShader& shader) override;
    private:
        shaderc_shader_kind MapStageToKind(VkShaderStageFlagBits stage) const;
    };
}