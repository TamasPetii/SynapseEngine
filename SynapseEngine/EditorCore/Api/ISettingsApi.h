#pragma once
#include "IApi.h"
#include "Engine/Scene/Settings/SceneSettings.h"
#include <vector>
#include <string>
#include <cstdint>

namespace Syn {
    class ISettingsApi : public IApi {
    public:
        virtual ~ISettingsApi() = default;

        virtual SceneSettings GetSceneSettings() const = 0;
        virtual void SetSceneSettings(const SceneSettings& settings) = 0;
        virtual std::vector<std::pair<uint32_t, std::string>> GetAvailableSkyTextures() const = 0;
    };
}