#pragma once
#include <variant>
#include <cstdint>

namespace Syn {
    struct ResizeMaterialViewportIntent {
        uint32_t width;
        uint32_t height;
    };

    using MaterialViewportIntent = std::variant<
        ResizeMaterialViewportIntent
    >;
}