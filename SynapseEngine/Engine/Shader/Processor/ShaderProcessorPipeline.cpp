#include "ShaderProcessorPipeline.h"

namespace Syn {
    void ShaderProcessorPipeline::AddProcessor(std::unique_ptr<IShaderProcessor> processor) {
        _processors.push_back(std::move(processor));
    }

    void ShaderProcessorPipeline::Run(CookedShader& cookedShader) {
        for (auto& processor : _processors) {
            processor->Process(cookedShader);
        }
    }
}