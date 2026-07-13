#pragma once
#include "Engine/Scene/Settings/SceneSettings.h"
#include <vector>
#include <string>
#include <cstdint>

namespace Syn 
{
    struct SettingsState
    {
        SceneSettings sceneSettings;
        std::vector<std::pair<uint32_t, std::string>> availableSkyTextures;
    };
}