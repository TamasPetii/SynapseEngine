#include "DefaultGpuShaderExtractor.h"
#include "Engine/Logger/SynLog.h"

namespace Syn {
    DefaultGpuShaderExtractor::DefaultGpuShaderExtractor(std::shared_ptr<IShaderReflectorRegistry> reflectorRegistry)
        : _reflectorRegistry(std::move(reflectorRegistry))
    {}

    void DefaultGpuShaderExtractor::Extract(GpuShaderData& gpuData) {
        if (gpuData.spirv.empty()) {
            Warning("GpuShaderData contains no SPIR-V data to reflect.");
            return;
        }

        IShaderReflector* reflector = _reflectorRegistry->GetReflector(ShaderBytecodeFormat::SPIRV);

        if (!reflector) {
            Error("No suitable reflector found for SPIR-V format.");
            return;
        }

        Info("Reflecting shader data...");

        gpuData.reflect = reflector->Reflect(gpuData.spirv, gpuData.stage, "GpuShader");
    }
}