#pragma once
#include "Engine/SynApi.h"
#include "IMeshProcessorPipeline.h"

namespace Syn
{
    class SYN_API MeshProcessorPipeline : public IMeshProcessorPipeline
    {
    public:
        MeshProcessorPipeline() = default;

        MeshProcessorPipeline(const MeshProcessorPipeline&) = delete;
        MeshProcessorPipeline& operator=(const MeshProcessorPipeline&) = delete;

        virtual void AddProcessor(std::unique_ptr<IMeshProcessor> processor) override;
        virtual void Run(CookedModel& cookedModel) override;
    private:
        std::vector<std::unique_ptr<IMeshProcessor>> _processors;
    };
}