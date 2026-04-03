#pragma once
#include "Engine/SynApi.h"
#include "IImageProcessorPipeline.h"

namespace Syn
{
    class SYN_API ImageProcessorPipeline : public IImageProcessorPipeline
    {
    public:
        ImageProcessorPipeline() = default;

        ImageProcessorPipeline(const ImageProcessorPipeline&) = delete;
        ImageProcessorPipeline& operator=(const ImageProcessorPipeline&) = delete;

        void AddProcessor(std::unique_ptr<IImageProcessor> processor) override;
        void Run(CookedImage& cookedImage) override;
    private:
        std::vector<std::unique_ptr<IImageProcessor>> _processors;
    };
}