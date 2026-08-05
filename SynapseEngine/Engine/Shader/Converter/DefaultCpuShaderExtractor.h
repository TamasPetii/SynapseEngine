#pragma once
#include "Engine/SynApi.h"
#include "ICpuShaderExtractor.h"
#include "Engine/Shader/Reflector/IShaderReflectorRegistry.h"
#include <memory>

namespace Syn {
    class SYN_API DefaultCpuShaderExtractor : public ICpuShaderExtractor {
    public:
        DefaultCpuShaderExtractor(std::shared_ptr<IShaderReflectorRegistry> reflectorRegistry);
        ~DefaultCpuShaderExtractor() override = default;

        void Extract(const GpuShaderData& gpuData, CpuShaderData& outCpuData) const override;
    private:
        std::shared_ptr<IShaderReflectorRegistry> _reflectorRegistry;
    };
}