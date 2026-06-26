#pragma once
#include "Engine/Scene/Settings/SceneSettings.h"
#include <variant>

namespace Syn {
    struct UpdateSceneSettingsIntent {
        SceneSettings newSettings;
    };

    using SettingsIntent = std::variant<
        UpdateSceneSettingsIntent
    >;
}