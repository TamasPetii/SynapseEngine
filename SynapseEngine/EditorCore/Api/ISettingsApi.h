#pragma once
#include "Engine/Scene/SceneSettings.h"

namespace Syn {
    class ISettingsAPI {
    public:
        virtual ~ISettingsAPI() = default;

        virtual SceneSettings GetSceneSettings() const = 0;
        virtual void SetSceneSettings(const SceneSettings& settings) = 0;
    };
}