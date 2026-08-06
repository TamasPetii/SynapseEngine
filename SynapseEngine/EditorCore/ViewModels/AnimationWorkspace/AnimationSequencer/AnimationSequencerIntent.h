#pragma once
#include <variant>
#include <cstdint>
#include <vector>

namespace Syn {
    struct ChangeSequencerFrameIntent { int32_t frame; };
    struct SelectSequencerTrackIntent { uint32_t trackIndex; };

    struct ToggleSequencerGroupIntent {
        uint32_t trackIndex;
        bool isOpen;
    };

    struct UpdateTrackKeysIntent {
        uint32_t trackIndex;
        std::vector<int32_t> positionFrames;
        std::vector<int32_t> rotationFrames;
        std::vector<int32_t> scaleFrames;
    };

    using AnimationSequencerIntent = std::variant<
        ChangeSequencerFrameIntent,
        SelectSequencerTrackIntent,
        ToggleSequencerGroupIntent,
        UpdateTrackKeysIntent
    >;
}