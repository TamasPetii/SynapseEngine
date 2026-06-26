#pragma once
#include <variant>
#include "Engine/Material/Material.h"

namespace Syn {
    //Todo intents

    struct UpdateMaterialPropertyIntent {
        Material updatedMaterial;
    };

    using MaterialPropertiesIntent = std::variant<
        UpdateMaterialPropertyIntent
    >;
}