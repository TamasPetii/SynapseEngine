#pragma once
#include "Engine/SynApi.h"
#include "Engine/Image/Loader/IImageLoaderRegistry.h"
#include "Engine/Image/Processor/IImageProcessorPipeline.h"
#include "Engine/Image/Converter/IImageCooker.h"
#include "Engine/Image/Converter/IGpuImageConverter.h"
#include "Engine/Image/Source/IImageSource.h"
#include "Engine/Image/Data/Gpu/GpuImage.h"

#include <memory>
#include <string>
#include <optional>

namespace Syn
{
    class SYN_API ImageBuilder
    {
    public:
        ImageBuilder(
            std::unique_ptr<IImageLoaderRegistry> registry,
            std::unique_ptr<IImageProcessorPipeline> pipeline,
            std::unique_ptr<IGpuImageConverter> converter,
            std::unique_ptr<IImageCooker> cooker
        );

        ImageBuilder(const ImageBuilder&) = delete;
        ImageBuilder& operator=(const ImageBuilder&) = delete;

        void RegisterLoader(std::shared_ptr<IImageLoader> loader, int priority = 0);
        void RegisterProcessor(std::unique_ptr<IImageProcessor> processor);

        std::optional<GpuImage> BuildFromFile(const std::string& filePath);
        std::optional<GpuImage> BuildFromSource(IImageSource& source);

    private:
        std::unique_ptr<IImageLoaderRegistry> _registry;
        std::unique_ptr<IImageProcessorPipeline> _pipeline;
        std::unique_ptr<IGpuImageConverter> _converter;
        std::unique_ptr<IImageCooker> _cooker;
    };
}