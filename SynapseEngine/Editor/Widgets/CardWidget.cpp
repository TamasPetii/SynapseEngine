#include "CardWidget.h"
#include "Editor/Manager/EditorIcons.h"
#include <string>

namespace Syn::UI {

    bool BeginCard(const char* label, const char* icon, bool& isOpen) {
        ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.12f, 0.12f, 0.12f, 0.6f));
        ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, 6.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(12.0f, 12.0f));

        ImGuiChildFlags childFlags = ImGuiChildFlags_Borders | ImGuiChildFlags_AlwaysUseWindowPadding;

        if (isOpen) {
            childFlags |= ImGuiChildFlags_AutoResizeY;
        }

        float textHeight = ImGui::GetTextLineHeight();
        float closedHeight = textHeight + 24.0f;
        float height = isOpen ? 0.0f : closedHeight;

        std::string childId = std::string("##Card_") + label;
        ImGui::BeginChild(childId.c_str(), ImVec2(0, height), childFlags, ImGuiWindowFlags_NoScrollbar);

        ImVec2 startPos = ImGui::GetCursorPos();
        float availX = ImGui::GetContentRegionAvail().x;

        if (availX <= 0.0f) {
            availX = 1.0f;
        }

        if (ImGui::InvisibleButton(childId.c_str(), ImVec2(availX, textHeight))) {
            isOpen = !isOpen;
        }

        ImGui::SetCursorPos(startPos);

        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.4f, 0.8f, 1.0f, 1.0f));
        ImGui::Text("%s  %s", icon, label);
        ImGui::PopStyleColor();

        const char* chevron = isOpen ? SYN_ICON_CHEVRON_UP : SYN_ICON_CHEVRON_DOWN;
        float chevronWidth = ImGui::CalcTextSize(chevron).x;
        ImGui::SameLine(availX - chevronWidth);
        ImGui::TextDisabled("%s", chevron);

        if (isOpen) {
            ImGui::Separator();
            ImGui::Spacing();
        }

        return isOpen;
    }

    void EndCard() {
        ImGui::EndChild();
        ImGui::PopStyleVar(2);
        ImGui::PopStyleColor();
        ImGui::Spacing();
    }

}