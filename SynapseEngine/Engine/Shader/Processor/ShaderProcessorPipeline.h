#pragma once
#include "Engine/SynApi.h"
#include "IShaderProcessorPipeline.h"

namespace Syn {
    class SYN_API ShaderProcessorPipeline : public IShaderProcessorPipeline {
    public:
        ShaderProcessorPipeline() = default;

        ShaderProcessorPipeline(const ShaderProcessorPipeline&) = delete;
        ShaderProcessorPipeline& operator=(const ShaderProcessorPipeline&) = delete;

        void AddProcessor(std::unique_ptr<IShaderProcessor> processor) override;
        void Run(CookedShader& cookedShader) override;
    private:
        std::vector<std::unique_ptr<IShaderProcessor>> _processors;
    };
}