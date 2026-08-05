#include "DefaultGpuShaderConverter.h"
#include "Engine/Logger/SynLog.h"

namespace Syn {
    DefaultGpuShaderConverter::DefaultGpuShaderConverter(std::shared_ptr<IShaderCompilerRegistry> compilerRegistry)
        : _compilerRegistry(std::move(compilerRegistry))
    {}

    GpuShaderData DefaultGpuShaderConverter::Convert(const CookedShader& cookedShader) {
        GpuShaderData gpuData;
        gpuData.stage = cookedShader.stage;

        IShaderCompiler* compiler = _compilerRegistry->GetCompiler(cookedShader.language);

        if (!compiler) {
            Error("No suitable compiler found for shader language in file: {}", cookedShader.identifier);
            return gpuData;
        }

        Info("Compiling shader to SPIR-V: {}", cookedShader.identifier);

        gpuData.spirv = compiler->Compile(cookedShader);

        return gpuData;
    }
}