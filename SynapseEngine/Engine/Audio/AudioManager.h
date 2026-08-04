#pragma once
#include "Engine/SynApi.h"
#include "Engine/Manager/BaseResourceManager.h"
#include "Engine/Audio/Builder/AudioBuilder.h"
#include "Engine/Audio/Data/Sound.h"
#include <memory>
#include <string>
#include <functional>

namespace Syn
{
    using AudioSourceFactory = std::function<std::unique_ptr<IAudioSource>()>;

    class SYN_API AudioManager : public BaseResourceManager<Sound>
    {
    public:
        AudioManager(std::shared_ptr<AudioBuilder> builder);
        ~AudioManager() override = default;

        uint32_t LoadAudioAsync(const std::string& filePath);
        uint32_t LoadAudioFromSourceAsync(const std::string& name, AudioSourceFactory factory);

        uint32_t LoadAudioSync(const std::string& filePath);
        uint32_t LoadAudioFromSourceSync(const std::string& name, AudioSourceFactory factory);

    protected:
        void StartGpuUpload(EntryType& entry) override;
        void FinalizeResource(EntryType& entry) override;
    private:
        std::shared_ptr<AudioBuilder> _builder;
    };
}