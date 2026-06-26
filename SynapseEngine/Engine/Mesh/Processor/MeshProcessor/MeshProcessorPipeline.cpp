#include "MeshProcessorPipeline.h"

namespace Syn 
{
    void MeshProcessorPipeline::AddProcessor(std::unique_ptr<IMeshProcessor> processor)
    {
        _processors.push_back(std::move(processor));
    }

    void MeshProcessorPipeline::Run(CookedModel& cookedModel)
    {
        for (auto& processor : _processors)
        {
            processor->Process(cookedModel);
        }
    }
}