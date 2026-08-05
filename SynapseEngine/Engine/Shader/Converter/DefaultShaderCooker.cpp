#include "DefaultShaderCooker.h"

namespace Syn {
    CookedShader DefaultShaderCooker::Cook(const RawShader& rawShader) {
        CookedShader cooked;
        cooked.sourceCode = rawShader.sourceCode;
        cooked.identifier = rawShader.identifier;
        cooked.stage = rawShader.stage;
        cooked.language = rawShader.language;
        cooked.defines = rawShader.defines;

        return cooked;
    }
}