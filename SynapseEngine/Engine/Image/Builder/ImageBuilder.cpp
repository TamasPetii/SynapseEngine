#include "ImageBuilder.h"
#include "Engine/Image/Source/File/FileImageSource.h"

namespace Syn
{
    ImageBuilder::ImageBuilder(
        std::unique_ptr<IImageLoaderRegistry> registry,
        std::unique_ptr<IImageProcessorPipeline> pipeline,
        std::unique_ptr<IGpuImageConverter> converter,
        std::unique_ptr<IImageCooker> cooker) :
        _registry(std::move(registry)),
        _pipeline(std::move(pipeline)),
        _converter(std::move(converter)),
        _cooker(std::move(cooker))
    {}

    void ImageBuilder::RegisterLoader(std::shared_ptr<IImageLoader> loader, int priority)
    {
        _registry->Register(loader, priority);
    }

    void ImageBuilder::RegisterProcessor(std::unique_ptr<IImageProcessor> processor)
    {
        _pipeline->AddProcessor(std::move(processor));
    }

    std::optional<GpuImage> ImageBuilder::BuildFromFile(const std::string& filePath)
    {
        std::string ext = std::filesystem::path(filePath).extension().string();
        IImageLoader* loader = _registry->GetLoaderForExtension(ext);

        if (!loader)
            return std::nullopt;

        FileImageSource source(filePath, loader);
        return BuildFromSource(source);
    }

    std::optional<GpuImage> ImageBuilder::BuildFromSource(IImageSource& source)
    {
        auto rawOpt = source.Produce();
        if (!rawOpt)
            return std::nullopt;

        CookedImage cooked = _cooker->Cook(std::move(rawOpt).value());

        _pipeline->Run(cooked);

        return _converter->Convert(cooked);
    }
}