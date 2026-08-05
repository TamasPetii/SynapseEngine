#pragma once
#include "../IShaderCompiler.h"
#include <slang.h>

namespace Syn {
    class SYN_API SlangShaderCompiler : public IShaderCompiler {
    public:
        SlangShaderCompiler() = default;
        ~SlangShaderCompiler() override = default;

        std::vector<uint32_t> Compile(const CookedShader& shader) override;
    private:
        SlangStage MapStageToSlang(VkShaderStageFlagBits stage) const;
    };
}