#pragma once
#include "Engine/SynApi.h"
#include "IShaderCompilerRegistry.h"
#include <unordered_map>
#include <memory>

namespace Syn {
    class SYN_API ShaderCompilerRegistry : public IShaderCompilerRegistry {
    public:
        ShaderCompilerRegistry() = default;
        ~ShaderCompilerRegistry() override = default;

        void Register(ShaderLanguage language, std::shared_ptr<IShaderCompiler> compiler) override;
        IShaderCompiler* GetCompiler(ShaderLanguage language) override;

    private:
        std::unordered_map<ShaderLanguage, std::shared_ptr<IShaderCompiler>> _compilers;
    };
}