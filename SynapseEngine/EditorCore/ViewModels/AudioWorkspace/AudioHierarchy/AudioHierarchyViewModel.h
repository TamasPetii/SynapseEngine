#pragma once
#include "EditorCore/ViewModels/IViewModel.h"
#include "AudioHierarchyState.h"
#include "AudioHierarchyIntent.h"
#include "EditorCore/Api/IAudioApi.h"
#include "EditorCore/Api/IPreviewApi.h"

namespace Syn {
    class AudioHierarchyViewModel : public IViewModel<AudioHierarchyState, AudioHierarchyIntent> {
    public:
        AudioHierarchyViewModel(IAudioApi* audioApi, IPreviewApi* previewApi);
        ~AudioHierarchyViewModel() override = default;

        const AudioHierarchyState& GetState() const override { return _state; }
        void SyncWithEngine() override;
        void Dispatch(const AudioHierarchyIntent& intent) override;

    private:
        void RebuildList();
    private:
        IAudioApi* _audioApi = nullptr;
        IPreviewApi* _previewApi = nullptr;
        AudioHierarchyState _state;

        bool _isDirty = true;
        uint64_t _lastEngineVersion = 0;
    };
}