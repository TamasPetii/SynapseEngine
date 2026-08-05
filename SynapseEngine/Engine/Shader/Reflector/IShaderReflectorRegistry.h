#pragma once
#include "Engine/SynApi.h"
#include "IShaderReflector.h"
#include "Engine/Shader/Data/Common/ShaderBytecodeFormat.h"
#include <memory>

namespace Syn {
    class SYN_API IShaderReflectorRegistry {
    public:
        virtual ~IShaderReflectorRegistry() = default;
        virtual void Register(ShaderBytecodeFormat format, std::shared_ptr<IShaderReflector> reflector) = 0;
        virtual IShaderReflector* GetReflector(ShaderBytecodeFormat format) = 0;
    };
}