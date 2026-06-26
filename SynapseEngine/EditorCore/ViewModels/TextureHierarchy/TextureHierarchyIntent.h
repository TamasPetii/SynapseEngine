#pragma once
#include <string>
#include <variant>

namespace Syn {
    struct TextureSelectIntent {
        uint32_t textureId;
    };

    struct TextureSetSearchQueryIntent {
        std::string query;
    };

    struct TextureRefreshIntent {};

    using TextureHierarchyIntent = std::variant<
        TextureSelectIntent,
        TextureSetSearchQueryIntent,
        TextureRefreshIntent
    >;
}