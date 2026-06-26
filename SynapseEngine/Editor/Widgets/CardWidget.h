#pragma once
#include <imgui.h>

namespace Syn::UI {
    bool BeginCard(const char* label, const char* icon, bool& isOpen);
    void EndCard();
}