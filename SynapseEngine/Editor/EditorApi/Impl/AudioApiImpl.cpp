#include "AudioApiImpl.h"
#include "Engine/Audio/Engine/IAudioEngine.h"
#include <filesystem>

namespace Syn {

    std::vector<AudioItemData> AudioApiImpl::GetAllAudios() const {
        if (!_audioManager) return {};

        std::vector<AudioItemData> result;
        auto paths = _audioManager->GetResourcePaths();

        for (uint32_t i = 0; i < paths.size(); ++i) {
            if (_audioManager->GetEntryState(i) == ResourceState::Ready) {
                std::filesystem::path p(paths[i]);
                result.push_back({ i, p.filename().string(), paths[i] });
            }
        }
        return result;
    }

    uint64_t AudioApiImpl::GetVersion() const {
        return _audioManager ? _audioManager->GetVersion() : 0;
    }

    void AudioApiImpl::SetSelected(uint32_t audioId) {
        _selectedAudioId = audioId;
    }

    uint32_t AudioApiImpl::GetSelected() const {
        return _selectedAudioId;
    }

    const CpuAudioData* AudioApiImpl::GetAudioCpuData(uint32_t audioId) const {
        if (!_audioManager || audioId == INVALID_AUDIO_ID) return nullptr;

        auto resource = _audioManager->GetResource(audioId);
        if (resource) {
            return &resource->cpuData;
        }

        return nullptr;
    }

    void AudioApiImpl::Play() {
        if (_selectedAudioId == INVALID_AUDIO_ID) return;
        auto data = GetAudioCpuData(_selectedAudioId);
        if (!data) return;

        if (auto audioEngine = ServiceLocator::Get<IAudioEngine>()) {
            audioEngine->PlayPreview(data);
        }
    }

    void AudioApiImpl::Pause() {
        if (auto audioEngine = ServiceLocator::Get<IAudioEngine>()) {
            audioEngine->PausePreview();
        }
    }

    void AudioApiImpl::Stop() {
        if (auto audioEngine = ServiceLocator::Get<IAudioEngine>()) {
            audioEngine->StopPreview();
        }
    }

    void AudioApiImpl::SetPlaybackTime(float timeInSeconds) {
        if (auto audioEngine = ServiceLocator::Get<IAudioEngine>()) {
            audioEngine->SetPreviewTime(timeInSeconds);
        }
    }

    bool AudioApiImpl::IsPlaying() const {
        if (auto audioEngine = ServiceLocator::Get<IAudioEngine>()) {
            return audioEngine->IsPreviewPlaying();
        }
        return false;
    }

    float AudioApiImpl::GetPlaybackTime() const {
        if (auto audioEngine = ServiceLocator::Get<IAudioEngine>()) {
            return audioEngine->GetPreviewTime();
        }
        return 0.0f;
    }

    float AudioApiImpl::GetDuration() const {
        if (_selectedAudioId == INVALID_AUDIO_ID) return 0.0f;
        auto data = GetAudioCpuData(_selectedAudioId);

        if (data && data->sampleRate > 0) {
            return static_cast<float>(data->totalFrames) / static_cast<float>(data->sampleRate);
        }
        return 0.0f;
    }
}