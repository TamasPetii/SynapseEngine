#pragma once
#include "Engine/Scene/SceneSettings.h"
#include <variant>

namespace Syn {
    struct UpdateSceneSettingsIntent {
        SceneSettings newSettings;
    };

    using SettingsIntent = std::variant<
        UpdateSceneSettingsIntent
    >;
}