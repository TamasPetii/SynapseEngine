#include "ShaderReflectorRegistry.h"

namespace Syn {
    void ShaderReflectorRegistry::Register(ShaderBytecodeFormat format, std::shared_ptr<IShaderReflector> reflector) {
        _reflectors[format] = std::move(reflector);
    }

    IShaderReflector* ShaderReflectorRegistry::GetReflector(ShaderBytecodeFormat format) {
        auto it = _reflectors.find(format);
        if (it != _reflectors.end()) {
            return it->second.get();
        }
        return nullptr;
    }
}