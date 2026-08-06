#pragma once
#include "EditorCore/Api/IAudioApi.h"
#include "Engine/Audio/AudioManager.h"

namespace Syn {
    class AudioApiImpl : public IAudioApi {
    public:
        AudioApiImpl(AudioManager* audioManager)
            : _audioManager(audioManager) {}

        std::vector<AudioItemData> GetAllAudios() const override;
        uint64_t GetVersion() const override;

        void SetSelected(uint32_t audioId) override;
        uint32_t GetSelected() const override;

        const CpuAudioData* GetAudioCpuData(uint32_t audioId) const override;

    private:
        AudioManager* _audioManager;
        uint32_t _selectedAudioId = INVALID_AUDIO_ID;
    };
}