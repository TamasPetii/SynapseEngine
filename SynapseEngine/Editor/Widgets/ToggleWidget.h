#pragma once
#include <imgui.h>

namespace Syn::UI {
    bool ToggleButton(const char* label, bool active, const ImVec2& size = ImVec2(0, 0));
}