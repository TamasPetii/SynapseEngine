#include "AudioBuilder.h"
#include "Engine/Audio/Source/FIle/FileAudioSource.h"

namespace Syn
{
    AudioBuilder::AudioBuilder(
        std::unique_ptr<IAudioLoaderRegistry> registry,
        std::unique_ptr<IAudioProcessorPipeline> pipeline,
        std::unique_ptr<IAudioCooker> cooker,
        std::unique_ptr<ICpuAudioExtractor> extractor) :
        _registry(std::move(registry)),
        _pipeline(std::move(pipeline)),
        _cooker(std::move(cooker)),
        _extractor(std::move(extractor))
    {}

    void AudioBuilder::RegisterLoader(std::shared_ptr<IAudioLoader> loader, int priority)
    {
        _registry->Register(loader, priority);
    }

    void AudioBuilder::RegisterProcessor(std::unique_ptr<IAudioProcessor> processor)
    {
        _pipeline->AddProcessor(std::move(processor));
    }

    std::shared_ptr<Sound> AudioBuilder::BuildFromFile(const std::string& filePath)
    {
        std::string ext = std::filesystem::path(filePath).extension().string();
        IAudioLoader* loader = _registry->GetLoaderForExtension(ext);

        if (!loader)
            return nullptr;

        FileAudioSource source(filePath, loader);
        return BuildFromSource(source);
    }

    std::shared_ptr<Sound> AudioBuilder::BuildFromSource(IAudioSource& source)
    {
        auto rawAudioOpt = source.Produce();

        if (!rawAudioOpt)
            return nullptr;

        auto sound = std::make_shared<Sound>();
        sound->transientCpuData = std::make_unique<CookedAudio>();

        *(sound->transientCpuData) = _cooker->Cook(std::move(rawAudioOpt).value());
        _pipeline->Run(*(sound->transientCpuData));
        _extractor->Extract(*(sound->transientCpuData), sound->cpuData);

        return sound;
    }
}