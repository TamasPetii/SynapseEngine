#pragma once
#include "Engine/Scene/SceneSettings.h"

namespace Syn {
    class ISettingsApi {
    public:
        virtual ~ISettingsApi() = default;

        virtual SceneSettings GetSceneSettings() const = 0;
        virtual void SetSceneSettings(const SceneSettings& settings) = 0;
    };
}