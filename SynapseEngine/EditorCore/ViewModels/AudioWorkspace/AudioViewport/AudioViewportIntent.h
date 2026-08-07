#pragma once
#include <variant>
#include <cstdint>

namespace Syn {
    struct AudioViewportPlayIntent {};
    struct AudioViewportPauseIntent {};
    struct AudioViewportStopIntent {};
    struct AudioViewportSeekIntent { float timeInSeconds; };

    using AudioViewportIntent = std::variant<
        AudioViewportPlayIntent,
        AudioViewportPauseIntent,
        AudioViewportStopIntent,
        AudioViewportSeekIntent
    >;
}