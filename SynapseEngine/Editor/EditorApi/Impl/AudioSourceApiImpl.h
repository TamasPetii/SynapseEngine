#pragma once
#include "EditorCore/Api/IAudioSourceApi.h"
#include "Engine/Scene/SceneManager.h"

namespace Syn {
    class AudioSourceApiImpl : public IAudioSourceApi {
    public:
        AudioSourceApiImpl(SceneManager* sm) : _sceneManager(sm) {}

        bool HasAudioSource(EntityID entity) const override;

        uint32_t GetAudioSourceSoundIndex(EntityID entity) const override;
        bool GetAudioSourcePlay(EntityID entity) const override;
        bool GetAudioSourceLoop(EntityID entity) const override;
        bool GetAudioSourceIsSpatialized(EntityID entity) const override;
        float GetAudioSourceVolume(EntityID entity) const override;
        float GetAudioSourcePitch(EntityID entity) const override;
        float GetAudioSourceMinDistance(EntityID entity) const override;
        float GetAudioSourceMaxDistance(EntityID entity) const override;

        void SetAudioSourceSoundIndex(EntityID entity, uint32_t soundIndex) override;
        void SetAudioSourcePlay(EntityID entity, bool play) override;
        void SetAudioSourceLoop(EntityID entity, bool loop) override;
        void SetAudioSourceIsSpatialized(EntityID entity, bool isSpatialized) override;
        void SetAudioSourceVolume(EntityID entity, float volume) override;
        void SetAudioSourcePitch(EntityID entity, float pitch) override;
        void SetAudioSourceMinDistance(EntityID entity, float minDistance) override;
        void SetAudioSourceMaxDistance(EntityID entity, float maxDistance) override;

    private:
        SceneManager* _sceneManager;
    };
}