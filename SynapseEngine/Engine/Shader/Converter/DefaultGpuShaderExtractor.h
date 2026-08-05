#pragma once
#include "Engine/SynApi.h"
#include "IGpuShaderExtractor.h"
#include "Engine/Shader/Reflector/IShaderReflectorRegistry.h"
#include <memory>

namespace Syn {
    class SYN_API DefaultGpuShaderExtractor : public IGpuShaderExtractor {
    public:
        DefaultGpuShaderExtractor(std::shared_ptr<IShaderReflectorRegistry> reflectorRegistry);
        ~DefaultGpuShaderExtractor() override = default;

        void Extract(GpuShaderData& gpuData) override;
    private:
        std::shared_ptr<IShaderReflectorRegistry> _reflectorRegistry;
    };
}