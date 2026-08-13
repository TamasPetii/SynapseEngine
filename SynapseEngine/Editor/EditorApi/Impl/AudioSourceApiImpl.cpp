// Copyright (C) 2026 Tamás Péter
// This file is part of SynapseEngine.
//
// SynapseEngine is free software: you can redistribute it and/or modify
// it under the terms of the GNU Affero General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// SynapseEngine is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License
// along with SynapseEngine. If not, see <https://www.gnu.org/licenses/>.

#include "AudioSourceApiImpl.h"
#include "../EditorApiUtils.h"
#include "Engine/Component/Audio/AudioSourceComponent.h"

namespace Syn {

    bool AudioSourceApiImpl::HasAudioSource(EntityID entity) const {
        return EditorApiUtils::HasComponent<AudioSourceComponent>(_sceneManager, entity);
    }

    uint32_t AudioSourceApiImpl::GetAudioSourceSoundIndex(EntityID entity) const {
        return EditorApiUtils::ReadComponent<AudioSourceComponent>(_sceneManager, entity, [](const auto& c) { return c.soundIndex; }, 0u);
    }

    bool AudioSourceApiImpl::GetAudioSourcePlay(EntityID entity) const {
        return EditorApiUtils::ReadComponent<AudioSourceComponent>(_sceneManager, entity, [](const auto& c) { return c.play; }, false);
    }

    bool AudioSourceApiImpl::GetAudioSourceLoop(EntityID entity) const {
        return EditorApiUtils::ReadComponent<AudioSourceComponent>(_sceneManager, entity, [](const auto& c) { return c.loop; }, false);
    }

    bool AudioSourceApiImpl::GetAudioSourceIsSpatialized(EntityID entity) const {
        return EditorApiUtils::ReadComponent<AudioSourceComponent>(_sceneManager, entity, [](const auto& c) { return c.isSpatialized; }, false);
    }

    float AudioSourceApiImpl::GetAudioSourceVolume(EntityID entity) const {
        return EditorApiUtils::ReadComponent<AudioSourceComponent>(_sceneManager, entity, [](const auto& c) { return c.volume; }, 1.0f);
    }

    float AudioSourceApiImpl::GetAudioSourcePitch(EntityID entity) const {
        return EditorApiUtils::ReadComponent<AudioSourceComponent>(_sceneManager, entity, [](const auto& c) { return c.pitch; }, 1.0f);
    }

    float AudioSourceApiImpl::GetAudioSourceMinDistance(EntityID entity) const {
        return EditorApiUtils::ReadComponent<AudioSourceComponent>(_sceneManager, entity, [](const auto& c) { return c.minDistance; }, 1.0f);
    }

    float AudioSourceApiImpl::GetAudioSourceMaxDistance(EntityID entity) const {
        return EditorApiUtils::ReadComponent<AudioSourceComponent>(_sceneManager, entity, [](const auto& c) { return c.maxDistance; }, 100.0f);
    }

    void AudioSourceApiImpl::SetAudioSourceSoundIndex(EntityID entity, uint32_t soundIndex) {
        EditorApiUtils::ModifyComponent<AudioSourceComponent>(_sceneManager, entity, [&](auto& c, auto pool) { c.soundIndex = soundIndex; });
    }

    void AudioSourceApiImpl::SetAudioSourcePlay(EntityID entity, bool play) {
        EditorApiUtils::ModifyComponent<AudioSourceComponent>(_sceneManager, entity, [&](auto& c, auto pool) { c.play = play; });
    }

    void AudioSourceApiImpl::SetAudioSourceLoop(EntityID entity, bool loop) {
        EditorApiUtils::ModifyComponent<AudioSourceComponent>(_sceneManager, entity, [&](auto& c, auto pool) { c.loop = loop; });
    }

    void AudioSourceApiImpl::SetAudioSourceIsSpatialized(EntityID entity, bool isSpatialized) {
        EditorApiUtils::ModifyComponent<AudioSourceComponent>(_sceneManager, entity, [&](auto& c, auto pool) { c.isSpatialized = isSpatialized; });
    }

    void AudioSourceApiImpl::SetAudioSourceVolume(EntityID entity, float volume) {
        EditorApiUtils::ModifyComponent<AudioSourceComponent>(_sceneManager, entity, [&](auto& c, auto pool) { c.volume = volume; });
    }

    void AudioSourceApiImpl::SetAudioSourcePitch(EntityID entity, float pitch) {
        EditorApiUtils::ModifyComponent<AudioSourceComponent>(_sceneManager, entity, [&](auto& c, auto pool) { c.pitch = pitch; });
    }

    void AudioSourceApiImpl::SetAudioSourceMinDistance(EntityID entity, float minDistance) {
        EditorApiUtils::ModifyComponent<AudioSourceComponent>(_sceneManager, entity, [&](auto& c, auto pool) { c.minDistance = minDistance; });
    }

    void AudioSourceApiImpl::SetAudioSourceMaxDistance(EntityID entity, float maxDistance) {
        EditorApiUtils::ModifyComponent<AudioSourceComponent>(_sceneManager, entity, [&](auto& c, auto pool) { c.maxDistance = maxDistance; });
    }
}