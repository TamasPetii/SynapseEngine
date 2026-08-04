#pragma once
#include "Engine/SynApi.h"
#include "Engine/Audio/Loader/IAudioLoaderRegistry.h"
#include "Engine/Audio/Processor/IAudioProcessorPipeline.h"
#include "Engine/Audio/Converter/IAudioCooker.h"
#include "Engine/Audio/Source/IAudioSource.h"
#include "Engine/Audio/Converter/ICpuAudioExtractor.h"
#include "Engine/Audio/Data/Sound.h"

#include <memory>
#include <string>
#include <filesystem>

namespace Syn
{
    class SYN_API AudioBuilder
    {
    public:
        AudioBuilder(
            std::unique_ptr<IAudioLoaderRegistry> registry,
            std::unique_ptr<IAudioProcessorPipeline> pipeline,
            std::unique_ptr<IAudioCooker> cooker,
            std::unique_ptr<ICpuAudioExtractor> extractor
        );

        AudioBuilder(const AudioBuilder&) = delete;
        AudioBuilder& operator=(const AudioBuilder&) = delete;

        void RegisterLoader(std::shared_ptr<IAudioLoader> loader, int priority = 0);
        void RegisterProcessor(std::unique_ptr<IAudioProcessor> processor);

        std::shared_ptr<Sound> BuildFromFile(const std::string& filePath);
        std::shared_ptr<Sound> BuildFromSource(IAudioSource& source);
        IAudioLoader* GetLoaderForExtension(const std::string& ext) const { return _registry->GetLoaderForExtension(ext); }
    private:
        std::unique_ptr<IAudioLoaderRegistry> _registry;
        std::unique_ptr<IAudioProcessorPipeline> _pipeline;
        std::unique_ptr<IAudioCooker> _cooker;
        std::unique_ptr<ICpuAudioExtractor> _extractor;
    };
}