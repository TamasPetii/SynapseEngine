#pragma once
#include <variant>
#include <cstdint>

namespace Syn {
    struct DummyTexturePropertyIntent {};

    using TexturePropertiesIntent = std::variant<
        DummyTexturePropertyIntent
    >;
}