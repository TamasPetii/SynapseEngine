#pragma once
#include "EditorCore/Api/IAudioListenerApi.h"
#include "Engine/Scene/SceneManager.h"

namespace Syn {
    class AudioListenerApiImpl : public IAudioListenerApi {
    public:
        AudioListenerApiImpl(SceneManager* sm) : _sceneManager(sm) {}

        bool HasAudioListener(EntityID entity) const override;
        bool GetAudioListenerActive(EntityID entity) const override;

        void SetAudioListenerActive(EntityID entity, bool active) override;

    private:
        SceneManager* _sceneManager;
    };
}