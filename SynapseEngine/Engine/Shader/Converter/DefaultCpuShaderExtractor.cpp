#include "DefaultCpuShaderExtractor.h"
#include "Engine/Logger/SynLog.h"

namespace Syn {
    DefaultCpuShaderExtractor::DefaultCpuShaderExtractor(std::shared_ptr<IShaderReflectorRegistry> reflectorRegistry)
        : _reflectorRegistry(std::move(reflectorRegistry))
    {}

    void DefaultCpuShaderExtractor::Extract(const GpuShaderData& gpuData, CpuShaderData& outCpuData) const {
        outCpuData.stage = gpuData.stage;

        if (gpuData.spirv.empty()) {
            Warning("GpuShaderData contains no SPIR-V data to reflect.");
            return;
        }

        IShaderReflector* reflector = _reflectorRegistry->GetReflector(ShaderBytecodeFormat::SPIRV);

        if (!reflector) {
            Error("No suitable reflector found for SPIR-V format.");
            return;
        }

        outCpuData.reflection = reflector->Reflect(gpuData.spirv, gpuData.stage, outCpuData.identifier);
    }
}