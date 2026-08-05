#pragma once
#include <variant>

namespace Syn
{
    struct SetAudioListenerActiveIntent {
        bool active;
    };

    using AudioListenerIntent = std::variant<SetAudioListenerActiveIntent>;
}