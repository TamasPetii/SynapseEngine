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

    std::shared_ptr<Texture> ImageBuilder::BuildFromFile(const std::string& filePath)
    {
        std::string ext = std::filesystem::path(filePath).extension().string();
        IImageLoader* loader = _registry->GetLoaderForExtension(ext);

        if (!loader)
            return nullptr;

        FileImageSource source(filePath, loader);
        return BuildFromSource(source);
    }

    std::shared_ptr<Texture> ImageBuilder::BuildFromSource(IImageSource& source)
    {
        auto rawTextureOpt = source.Produce();

        if (!rawTextureOpt)
            return nullptr;

		auto texture = std::make_shared<Texture>();
		texture->cpuData = _cooker->Cook(std::move(rawTextureOpt).value());

        _pipeline->Run(texture->cpuData);

        texture->gpuData = _converter->Convert(texture->cpuData);

        return texture;
    }
}