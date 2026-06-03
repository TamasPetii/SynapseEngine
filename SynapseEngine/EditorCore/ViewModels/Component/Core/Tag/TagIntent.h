#pragma once
#include <string>
#include <variant>

namespace Syn {
    struct SetEntityNameIntent { 
        std::string newName;
    };

    struct SetEntityTagIntent {
        std::string newTag;
    };

    struct ToggleEntityIntent { 
        bool isEnabled;
    };

    using TagIntent = std::variant<
        SetEntityNameIntent,
        SetEntityTagIntent,
        ToggleEntityIntent
    >;
}