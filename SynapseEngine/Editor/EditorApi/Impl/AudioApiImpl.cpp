#include "AudioApiImpl.h"
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
}