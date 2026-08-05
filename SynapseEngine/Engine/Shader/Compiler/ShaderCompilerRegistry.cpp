#include "ShaderCompilerRegistry.h"

namespace Syn {
    void ShaderCompilerRegistry::Register(ShaderLanguage language, std::shared_ptr<IShaderCompiler> compiler) {
        _compilers[language] = std::move(compiler);
    }

    IShaderCompiler* ShaderCompilerRegistry::GetCompiler(ShaderLanguage language) {
        auto it = _compilers.find(language);
        if (it != _compilers.end()) {
            return it->second.get();
        }
        return nullptr;
    }
}