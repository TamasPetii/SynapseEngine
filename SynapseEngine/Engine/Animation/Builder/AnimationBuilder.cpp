#include "AnimationBuilder.h"
#include "Engine/Animation/Source/File/FileAnimationSource.h"

namespace Syn
{
    AnimationBuilder::AnimationBuilder(
        std::unique_ptr<IAnimationLoaderRegistry> registry,
        std::unique_ptr<IAnimationProcessorPipeline> pipeline,
        std::unique_ptr<IGpuAnimationConverter> converter,
        std::unique_ptr<IAnimationCooker> cooker) :
        _registry(std::move(registry)),
        _cooker(std::move(cooker)),
        _pipeline(std::move(pipeline)),
        _converter(std::move(converter))
    {}

    void AnimationBuilder::RegisterLoader(std::shared_ptr<IAnimationLoader> loader, int priority)
    {
        _registry->Register(loader, priority);
    }

    void AnimationBuilder::RegisterProcessor(std::unique_ptr<IAnimationProcessor> processor)
    {
        _pipeline->AddProcessor(std::move(processor));
    }

    std::shared_ptr<Animation> AnimationBuilder::BuildFromFile(const std::string& filePath, const CpuModelData& baseModel)
    {
        std::string ext = std::filesystem::path(filePath).extension().string();
        IAnimationLoader* loader = _registry->GetLoaderForExtension(ext);

        if (!loader)
            return nullptr;

        FileAnimationSource source(filePath, loader);
        return BuildFromSource(source, baseModel);
    }

    std::shared_ptr<Animation> AnimationBuilder::BuildFromSource(IAnimationSource& source, const CpuModelData& baseModel)
    {
        auto rawAnimOpt = source.Produce();

        if (!rawAnimOpt)
            return nullptr;

        auto animation = std::make_shared<Animation>();
        animation->transientCpuData = std::make_unique<CookedAnimation>();
        animation->transientGpuData = std::make_unique<GpuBatchedAnimation>();

        *(animation->transientCpuData) = _cooker->Cook(std::move(rawAnimOpt).value());
        _pipeline->Run(*(animation->transientCpuData), baseModel);
        *(animation->transientGpuData) = _converter->Convert(*(animation->transientCpuData), baseModel);

        return animation;
    }
}