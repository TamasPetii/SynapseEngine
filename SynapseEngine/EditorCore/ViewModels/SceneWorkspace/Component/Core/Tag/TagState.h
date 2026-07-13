#pragma once
#include <string>

namespace Syn {
    struct TagState
    {
        std::string name = "Entity";
        std::string tag = "Untagged";
        bool isEnabled = true;
    };
}