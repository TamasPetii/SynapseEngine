#pragma once
#include <variant>

namespace Syn {
    struct ModelPropertiesRefreshIntent {};

    using ModelPropertiesIntent = std::variant<
        ModelPropertiesRefreshIntent
    >;
}