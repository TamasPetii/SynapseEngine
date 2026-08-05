#pragma once
#include "Engine/SynApi.h"
#include "IShaderCompiler.h"
#include "Engine/Shader/Data/Common/ShaderLanguageType.h"
#include <memory>

namespace Syn {
    class SYN_API IShaderCompilerRegistry {
    public:
        virtual ~IShaderCompilerRegistry() = default;
        virtual void Register(ShaderLanguage language, std::shared_ptr<IShaderCompiler> compiler) = 0;
        virtual IShaderCompiler* GetCompiler(ShaderLanguage language) = 0;
    };
}