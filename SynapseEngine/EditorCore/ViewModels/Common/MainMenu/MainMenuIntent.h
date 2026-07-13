#pragma once
#include <variant>

namespace Syn {
    struct NewSceneIntent {};
    struct LoadSceneIntent {};
    struct SaveSceneIntent {};

    using MainMenuIntent = std::variant<
        NewSceneIntent,
        LoadSceneIntent,
        SaveSceneIntent
    >;
}