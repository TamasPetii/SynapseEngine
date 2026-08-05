#pragma once
#include "EditorCore/Types/EntityHandle.h"
#include "IApi.h"
#include <cstdint>

namespace Syn {
    class IAudioSourceApi : public IApi {
    public:
        virtual ~IAudioSourceApi() = default;

        virtual bool HasAudioSource(EntityID entity) const = 0;

        virtual uint32_t GetAudioSourceSoundIndex(EntityID entity) const = 0;
        virtual bool GetAudioSourcePlay(EntityID entity) const = 0;
        virtual bool GetAudioSourceLoop(EntityID entity) const = 0;
        virtual bool GetAudioSourceIsSpatialized(EntityID entity) const = 0;
        virtual float GetAudioSourceVolume(EntityID entity) const = 0;
        virtual float GetAudioSourcePitch(EntityID entity) const = 0;
        virtual float GetAudioSourceMinDistance(EntityID entity) const = 0;
        virtual float GetAudioSourceMaxDistance(EntityID entity) const = 0;

        virtual void SetAudioSourceSoundIndex(EntityID entity, uint32_t soundIndex) = 0;
        virtual void SetAudioSourcePlay(EntityID entity, bool play) = 0;
        virtual void SetAudioSourceLoop(EntityID entity, bool loop) = 0;
        virtual void SetAudioSourceIsSpatialized(EntityID entity, bool isSpatialized) = 0;
        virtual void SetAudioSourceVolume(EntityID entity, float volume) = 0;
        virtual void SetAudioSourcePitch(EntityID entity, float pitch) = 0;
        virtual void SetAudioSourceMinDistance(EntityID entity, float minDistance) = 0;
        virtual void SetAudioSourceMaxDistance(EntityID entity, float maxDistance) = 0;
    };
}