#pragma once
#include "Engine/SynApi.h"

#include "../Loader/IAnimationLoader.h"
#include "../Loader/IAnimationLoaderRegistry.h"
#include "../Processor/IAnimationProcessor.h"
#include "../Processor/IAnimationProcessorPipeline.h"
#include "../Source/IAnimationSource.h"
#include "../Data/Animation.h"

#include "../Converter/IAnimationCooker.h"
#include "../Converter/IGpuAnimationConverter.h"
#include "../Uploader/IGpuAnimationUploader.h"

#include <memory>
#include <string>

namespace Syn
{
    class SYN_API AnimationBuilder
    {
    public:
        AnimationBuilder(
            std::unique_ptr<IAnimationLoaderRegistry> registry,
            std::unique_ptr<IAnimationProcessorPipeline> pipeline,
            std::unique_ptr<IGpuAnimationConverter> converter,
            std::unique_ptr<IAnimationCooker> cooker
        );

        AnimationBuilder(const AnimationBuilder&) = delete;
        AnimationBuilder& operator=(const AnimationBuilder&) = delete;

        void RegisterLoader(std::shared_ptr<IAnimationLoader> loader, int priority = 0);
        void RegisterProcessor(std::unique_ptr<IAnimationProcessor> processor);

        std::shared_ptr<Animation> BuildFromFile(const std::string& filePath, const CookedModel& baseModel);
        std::shared_ptr<Animation> BuildFromSource(IAnimationSource& source, const CookedModel& baseModel);
    private:
        std::unique_ptr<IAnimationLoaderRegistry> _registry;
        std::unique_ptr<IAnimationProcessorPipeline> _pipeline;
        std::unique_ptr<IGpuAnimationConverter> _converter;
        std::unique_ptr<IAnimationCooker> _cooker;
    };
}