#include "PropertyGrid.h"
#include <imgui_internal.h>
#include "Editor/Manager/EditorIcons.h"

namespace Syn::UI {

    bool BeginPropertyGrid(const char* id) {
        ImGuiTableFlags flags = ImGuiTableFlags_BordersInnerV | ImGuiTableFlags_Resizable | ImGuiTableFlags_SizingStretchProp;
        ImGui::PushStyleVar(ImGuiStyleVar_CellPadding, ImVec2(4.0f, 4.0f));

        bool isOpen = ImGui::BeginTable(id, 2, flags);
        if (isOpen) {
            ImGui::TableSetupColumn("Label", ImGuiTableColumnFlags_WidthFixed);
            ImGui::TableSetupColumn("Value", ImGuiTableColumnFlags_WidthStretch);
        }
        else {
            ImGui::PopStyleVar();
        }

        return isOpen;
    }

    void EndPropertyGrid() {
        ImGui::EndTable();
        ImGui::PopStyleVar();
    }

    void PropertySeparator() {
        ImGui::TableNextRow();
        ImGui::TableSetColumnIndex(0);
        ImGui::Separator();
        ImGui::TableSetColumnIndex(1);
        ImGui::Separator();
    }

    void BeginProperty(const char* label, int indentLevel) {
        ImGui::TableNextRow();
        ImGui::TableSetColumnIndex(0);
        ImGui::AlignTextToFramePadding();

        if (indentLevel > 0) {
            float indentSize = indentLevel * 16.0f;
            ImGui::Indent(indentSize);

            ImGui::TextDisabled(SYN_ICON_LEVEL_DOWN_ALT);
            ImGui::SameLine(0, 4.0f);
        }

        ImGui::TextUnformatted(label, ImGui::FindRenderedTextEnd(label));

        if (indentLevel > 0) {
            ImGui::Unindent(indentLevel * 16.0f);
        }

        ImGui::TableSetColumnIndex(1);
        ImGui::SetNextItemWidth(-FLT_MIN);
    }

    bool PropertyDragFloat(const char* label, float& value, float v_speed, float v_min, float v_max, const char* format, int indentLevel) {
        BeginProperty(label, indentLevel);
        std::string widgetId = std::string("##") + label;
        bool changed = ImGui::DragFloat(widgetId.c_str(), &value, v_speed, v_min, v_max, format);
        return changed || ImGui::IsItemDeactivatedAfterEdit();
    }

    bool PropertyDragFloat2(const char* label, glm::vec2& values, float v_speed, float v_min, float v_max, const char* format, int indentLevel) {
        BeginProperty(label, indentLevel);
        std::string widgetId = std::string("##") + label;
        ImGui::BeginGroup();
        bool changed = ImGui::DragFloat2(widgetId.c_str(), &values.x, v_speed, v_min, v_max, format);
        ImGui::EndGroup();
        return changed || ImGui::IsItemDeactivatedAfterEdit();
    }

    bool PropertyDragFloat3(const char* label, glm::vec3& values, float v_speed, float v_min, float v_max, const char* format, int indentLevel) {
        BeginProperty(label, indentLevel);
        std::string widgetId = std::string("##") + label;
        ImGui::BeginGroup();
        bool changed = ImGui::DragFloat3(widgetId.c_str(), &values.x, v_speed, v_min, v_max, format);
        ImGui::EndGroup();
        return changed || ImGui::IsItemDeactivatedAfterEdit();
    }

    bool PropertyDragFloat4(const char* label, glm::vec4& values, float v_speed, float v_min, float v_max, const char* format, int indentLevel) {
        BeginProperty(label, indentLevel);
        std::string widgetId = std::string("##") + label;
        ImGui::BeginGroup();
        bool changed = ImGui::DragFloat4(widgetId.c_str(), &values.x, v_speed, v_min, v_max, format);
        ImGui::EndGroup();
        return changed || ImGui::IsItemDeactivatedAfterEdit();
    }

    bool PropertySliderFloat(const char* label, float& value, float v_min, float v_max, const char* format, int indentLevel) {
        BeginProperty(label, indentLevel);
        std::string widgetId = std::string("##") + label;
        bool changed = ImGui::SliderFloat(widgetId.c_str(), &value, v_min, v_max, format);
        return changed || ImGui::IsItemDeactivatedAfterEdit();
    }

    bool PropertySliderFloat2(const char* label, glm::vec2& values, float v_min, float v_max, const char* format, int indentLevel) {
        BeginProperty(label, indentLevel);
        std::string widgetId = std::string("##") + label;
        ImGui::BeginGroup();
        bool changed = ImGui::SliderFloat2(widgetId.c_str(), &values.x, v_min, v_max, format);
        ImGui::EndGroup();
        return changed || ImGui::IsItemDeactivatedAfterEdit();
    }

    bool PropertySliderFloat3(const char* label, glm::vec3& values, float v_min, float v_max, const char* format, int indentLevel) {
        BeginProperty(label, indentLevel);
        std::string widgetId = std::string("##") + label;
        ImGui::BeginGroup();
        bool changed = ImGui::SliderFloat3(widgetId.c_str(), &values.x, v_min, v_max, format);
        ImGui::EndGroup();
        return changed || ImGui::IsItemDeactivatedAfterEdit();
    }

    bool PropertySliderFloat4(const char* label, glm::vec4& values, float v_min, float v_max, const char* format, int indentLevel) {
        BeginProperty(label, indentLevel);
        std::string widgetId = std::string("##") + label;
        ImGui::BeginGroup();
        bool changed = ImGui::SliderFloat4(widgetId.c_str(), &values.x, v_min, v_max, format);
        ImGui::EndGroup();
        return changed || ImGui::IsItemDeactivatedAfterEdit();
    }

    bool PropertyColor3(const char* label, glm::vec3& color, int indentLevel) {
        BeginProperty(label, indentLevel);
        std::string widgetId = std::string("##") + label;
        bool changed = ImGui::ColorEdit3(widgetId.c_str(), &color.x);
        return changed || ImGui::IsItemDeactivatedAfterEdit();
    }

    bool PropertyColor4(const char* label, glm::vec4& color, int indentLevel) {
        BeginProperty(label, indentLevel);
        std::string widgetId = std::string("##") + label;
        bool changed = ImGui::ColorEdit4(widgetId.c_str(), &color.x);
        return changed || ImGui::IsItemDeactivatedAfterEdit();
    }

    bool PropertyCheckbox(const char* label, bool& value, int indentLevel) {
        BeginProperty(label, indentLevel);
        std::string widgetId = std::string("##") + label;
        return ImGui::Checkbox(widgetId.c_str(), &value);
    }

    bool BeginPropertyCombo(const char* label, const char* preview_value, int indentLevel) {
        BeginProperty(label, indentLevel);
        std::string widgetId = std::string("##") + label;
        return ImGui::BeginCombo(widgetId.c_str(), preview_value);
    }

    void EndPropertyCombo() {
        ImGui::EndCombo();
    }

    void PropertyText(const char* label, const char* text, int indentLevel) {
        BeginProperty(label, indentLevel);
        ImGui::TextUnformatted(text);
    }
}