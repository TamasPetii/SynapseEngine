#pragma once
#include "EditorCore/Types/EntityHandle.h"
#include "IApi.h"

namespace Syn {
    class IAudioListenerApi : public IApi {
    public:
        virtual ~IAudioListenerApi() = default;

        virtual bool HasAudioListener(EntityID entity) const = 0;
        virtual bool GetAudioListenerActive(EntityID entity) const = 0;

        virtual void SetAudioListenerActive(EntityID entity, bool active) = 0;
    };
}