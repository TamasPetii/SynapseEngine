#pragma once
#include <variant>
#include <cstdint>

namespace Syn {
    struct ChangeSequencerFrameIntent { int32_t frame; };
    struct SelectSequencerTrackIntent { uint32_t trackIndex; };

    using AnimationSequencerIntent = std::variant<
        ChangeSequencerFrameIntent,
        SelectSequencerTrackIntent
    >;
}