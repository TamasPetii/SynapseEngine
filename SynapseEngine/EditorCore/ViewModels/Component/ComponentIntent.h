#pragma once
#include "Core/Tag/TagIntent.h"
#include "Core/Transform/TransformIntent.h"
#include "Light/DirectionLight/DirectionLightIntent.h"
#include "Light/PointLight/PointLightIntent.h"
#include "Light/SpotLight/SpotLightIntent.h"
#include <variant>

namespace Syn {
    using ComponentIntent = std::variant<
        TagIntent,
        TransformIntent,
        DirectionLightIntent,
        PointLightIntent,
        SpotLightIntent
    >;
}