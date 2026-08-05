#pragma once
#include "Engine/SynApi.h"
#include "IGpuShaderConverter.h"
#include "Engine/Shader/Compiler/IShaderCompilerRegistry.h"
#include <memory>

namespace Syn {
    class SYN_API DefaultGpuShaderConverter : public IGpuShaderConverter {
    public:
        DefaultGpuShaderConverter(std::shared_ptr<IShaderCompilerRegistry> compilerRegistry);
        ~DefaultGpuShaderConverter() override = default;

        GpuShaderData Convert(const CookedShader& cookedShader) override;
    private:
        std::shared_ptr<IShaderCompilerRegistry> _compilerRegistry;
    };
}