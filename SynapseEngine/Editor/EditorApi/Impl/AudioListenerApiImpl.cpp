#include "AudioListenerApiImpl.h"
#include "../EditorApiUtils.h"
#include "Engine/Component/Audio/AudioListenerComponent.h"

namespace Syn {

    bool AudioListenerApiImpl::HasAudioListener(EntityID entity) const {
        return EditorApiUtils::HasComponent<AudioListenerComponent>(_sceneManager, entity);
    }

    bool AudioListenerApiImpl::GetAudioListenerActive(EntityID entity) const {
        return EditorApiUtils::ReadComponent<AudioListenerComponent>(_sceneManager, entity, [](const auto& c) { return c.active; }, true);
    }

    void AudioListenerApiImpl::SetAudioListenerActive(EntityID entity, bool active) {
        EditorApiUtils::ModifyComponent<AudioListenerComponent>(_sceneManager, entity, [&](auto& c, auto pool) { c.active = active; });
    }
}