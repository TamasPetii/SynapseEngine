#include "AudioHierarchyViewModel.h"
#include "Editor/Manager/EditorIcons.h"
#include "Engine/Manager/PreviewManager.h"
#include <algorithm>

namespace Syn {
    AudioHierarchyViewModel::AudioHierarchyViewModel(IAudioApi* audioApi, IPreviewApi* previewApi)
        : _audioApi(audioApi), _previewApi(previewApi)
    {}

    void AudioHierarchyViewModel::SyncWithEngine() {
        if (!_audioApi) return;

        if (_previewApi) {
            _state.atlasHandle = _previewApi->GetAtlasHandle();
        }

        uint64_t currentVersion = _audioApi->GetVersion();
        uint32_t currentSelection = _audioApi->GetSelected();

        if (currentVersion != _lastEngineVersion || _isDirty) {
            RebuildList();
            _lastEngineVersion = currentVersion;
            _isDirty = false;
        }

        if (_previewApi) {
            for (auto& node : _state.filteredNodes) {
                if (_previewApi->HasPreview(PreviewResourceType::Audio, node.id)) {
                    node.hasPreview = true;
                    _previewApi->GetPreviewUVs(PreviewResourceType::Audio, node.id, node.uv0, node.uv1);
                }
            }
        }

        if (_state.selectedAudioId != currentSelection) {
            _state.selectedAudioId = currentSelection;
        }
    }

    void AudioHierarchyViewModel::Dispatch(const AudioHierarchyIntent& intent) {
        std::visit([this](auto&& arg) {
            using T = std::decay_t<decltype(arg)>;

            if constexpr (std::is_same_v<T, AudioHierarchySelectIntent>) {
                if (_audioApi) {
                    _audioApi->SetSelected(arg.audioId);
                }
            }
            else if constexpr (std::is_same_v<T, AudioHierarchySetSearchIntent>) {
                if (_state.searchQuery != arg.query) {
                    _state.searchQuery = arg.query;
                    _isDirty = true;
                }
            }
            else if constexpr (std::is_same_v<T, AudioHierarchyRefreshIntent>) {
                _isDirty = true;
            }
            }, intent);
    }

    void AudioHierarchyViewModel::RebuildList() {
        if (!_audioApi) return;

        _state.filteredNodes.clear();
        auto allAudios = _audioApi->GetAllAudios();

        std::string searchLower = _state.searchQuery;
        std::transform(searchLower.begin(), searchLower.end(), searchLower.begin(), ::tolower);

        for (const auto& audio : allAudios) {
            std::string nameLower = audio.name;
            std::transform(nameLower.begin(), nameLower.end(), nameLower.begin(), ::tolower);

            if (searchLower.empty() || nameLower.find(searchLower) != std::string::npos) {
                AudioHierarchyNode node;
                node.id = audio.id;
                node.name = audio.name;
                node.path = audio.path;
                node.icon = SYN_ICON_AUDIO;
                _state.filteredNodes.push_back(node);
            }
        }
    }
}