#include "ImageProcessorPipeline.h"

namespace Syn
{
    void ImageProcessorPipeline::AddProcessor(std::unique_ptr<IImageProcessor> processor)
    {
        _processors.push_back(std::move(processor));
    }

    void ImageProcessorPipeline::Run(CookedImage& cookedImage)
    {
        for (auto& processor : _processors)
        {
            processor->Process(cookedImage);
        }
    }
}