#pragma once
#include <variant>

namespace Syn {
    struct DummyTextureGraphIntent {};

    using TextureGraphIntent = std::variant<
        DummyTextureGraphIntent
    >;
}