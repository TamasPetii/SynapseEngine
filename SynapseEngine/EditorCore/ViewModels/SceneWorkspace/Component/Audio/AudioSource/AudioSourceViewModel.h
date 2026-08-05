#pragma once
#include "EditorCore/ViewModels/IViewModel.h"
#include "EditorCore/Interaction/DragInteraction.h"
#include "AudioSourceState.h"
#include "AudioSourceIntent.h"
#include "AudioSourceCommands.h"
#include "EditorCore/Api/ISelectionApi.h"
#include "EditorCore/Api/IAudioSourceApi.h"

namespace Syn {
    class AudioSourceViewModel : public IViewModel<AudioSourceState, AudioSourceIntent> {
    public:
        AudioSourceViewModel(ISelectionApi* selectionApi, IAudioSourceApi* audioSourceApi);
        ~AudioSourceViewModel() override = default;

        const AudioSourceState& GetState() const override;
        void SyncWithEngine() override;
        void Dispatch(const AudioSourceIntent& intent) override;

    private:
        void HandleSetSoundIndex(const SetAudioSourceSoundIndexIntent& intent);
        void HandleSetPlay(const SetAudioSourcePlayIntent& intent);
        void HandleSetLoop(const SetAudioSourceLoopIntent& intent);
        void HandleSetIsSpatialized(const SetAudioSourceIsSpatializedIntent& intent);
        void HandleSetVolume(const SetAudioSourceVolumeIntent& intent);
        void HandleSetPitch(const SetAudioSourcePitchIntent& intent);
        void HandleSetMinDistance(const SetAudioSourceMinDistanceIntent& intent);
        void HandleSetMaxDistance(const SetAudioSourceMaxDistanceIntent& intent);

    private:
        ISelectionApi* _selectionApi = nullptr;
        IAudioSourceApi* _audioSourceApi = nullptr;
        AudioSourceState _state;

        DragInteraction<float> _volumeDrag;
        DragInteraction<float> _pitchDrag;
        DragInteraction<float> _minDistanceDrag;
        DragInteraction<float> _maxDistanceDrag;
    };
}