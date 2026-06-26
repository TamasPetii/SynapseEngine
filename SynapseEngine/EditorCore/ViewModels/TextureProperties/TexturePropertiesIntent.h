#pragma once
#include <variant>

namespace Syn {
    struct DummyTexturePropertyIntent {};

    using TexturePropertiesIntent = std::variant<
        DummyTexturePropertyIntent
    >;
}