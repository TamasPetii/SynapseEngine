#pragma once
#include "Engine/SynApi.h"
#include "IShaderReflectorRegistry.h"
#include <unordered_map>

namespace Syn {
    class SYN_API ShaderReflectorRegistry : public IShaderReflectorRegistry {
    public:
        ShaderReflectorRegistry() = default;
        ~ShaderReflectorRegistry() override = default;

        void Register(ShaderBytecodeFormat format, std::shared_ptr<IShaderReflector> reflector) override;
        IShaderReflector* GetReflector(ShaderBytecodeFormat format) override;

    private:
        std::unordered_map<ShaderBytecodeFormat, std::shared_ptr<IShaderReflector>> _reflectors;
    };
}