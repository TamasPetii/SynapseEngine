#pragma once
#include "Core/Tag/TagIntent.h"
#include "Core/Transform/TransformIntent.h"
#include <variant>

namespace Syn {
    using ComponentIntent = std::variant<
        TagIntent,
        TransformIntent
    >;
}