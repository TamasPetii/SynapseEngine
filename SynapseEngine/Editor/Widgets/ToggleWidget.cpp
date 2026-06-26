#include "ToggleWidget.h"
#include <imgui_internal.h>

namespace Syn::UI {

    bool ToggleButton(const char* label, bool active, const ImVec2& size) {
        ImVec4* colors = ImGui::GetStyle().Colors;
        ImVec4 activeColor = colors[ImGuiCol_ButtonActive];

        if (active) {
            ImGui::PushStyleColor(ImGuiCol_Button, activeColor);
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, activeColor);
            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 1.0f, 1.0f));
        }
        else {
            ImGui::PushStyleColor(ImGuiCol_Button, colors[ImGuiCol_FrameBg]);
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, colors[ImGuiCol_FrameBgHovered]);
            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.6f, 0.6f, 0.6f, 1.0f));
        }

        bool clicked = ImGui::Button(label, size);

        ImGui::PopStyleColor(3);
        return clicked;
    }

}