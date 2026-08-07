#pragma once
#include "Engine/SynApi.h"
#include "Engine/Audio/Data/Cpu/CpuAudioData.h"
#include "Engine/Registry/Registry.h"
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

namespace Syn
{
    struct AudioSourceComponent;

    class SYN_API IAudioEngine
    {
    public:
        virtual ~IAudioEngine() = default;

        virtual void Init() = 0;
        virtual void Shutdown() = 0;

        virtual void SetListenerTransform(const glm::vec3& position, const glm::vec3& forward, const glm::vec3& up) = 0;
        virtual void UpdateSound(EntityID entity, const AudioSourceComponent& component, const CpuAudioData& audioData, const glm::vec3& position) = 0;
        virtual void StopSound(EntityID entity) = 0;
        virtual void StopAllSounds() = 0;

        virtual void PlayPreview(const CpuAudioData* audioData) = 0;
        virtual void PausePreview() = 0;
        virtual void StopPreview() = 0;
        virtual void SetPreviewTime(float timeInSeconds) = 0;
        virtual bool IsPreviewPlaying() const = 0;
        virtual float GetPreviewTime() const = 0;
    };
}