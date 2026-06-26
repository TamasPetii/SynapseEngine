#pragma once
#include <imgui.h>
#include <glm/glm.hpp>
#include <string>

namespace Syn::UI {
    bool BeginPropertyGrid(const char* id);
    void EndPropertyGrid();

    void PropertySeparator();
    void BeginProperty(const char* label, int indentLevel = 0);

    bool PropertyDragFloat(const char* label, float& value, float v_speed = 1.0f, float v_min = 0.0f, float v_max = 0.0f, const char* format = "%.3f", int indentLevel = 0);
    bool PropertyDragFloat2(const char* label, glm::vec2& values, float v_speed = 1.0f, float v_min = 0.0f, float v_max = 0.0f, const char* format = "%.3f", int indentLevel = 0);
    bool PropertyDragFloat3(const char* label, glm::vec3& values, float v_speed = 1.0f, float v_min = 0.0f, float v_max = 0.0f, const char* format = "%.3f", int indentLevel = 0);
    bool PropertyDragFloat4(const char* label, glm::vec4& values, float v_speed = 1.0f, float v_min = 0.0f, float v_max = 0.0f, const char* format = "%.3f", int indentLevel = 0);

    bool PropertySliderFloat(const char* label, float& value, float v_min, float v_max, const char* format = "%.3f", int indentLevel = 0);
    bool PropertySliderFloat2(const char* label, glm::vec2& values, float v_min, float v_max, const char* format = "%.3f", int indentLevel = 0);
    bool PropertySliderFloat3(const char* label, glm::vec3& values, float v_min, float v_max, const char* format = "%.3f", int indentLevel = 0);
    bool PropertySliderFloat4(const char* label, glm::vec4& values, float v_min, float v_max, const char* format = "%.3f", int indentLevel = 0);

    bool PropertyColor3(const char* label, glm::vec3& color, int indentLevel = 0);
    bool PropertyColor4(const char* label, glm::vec4& color, int indentLevel = 0);

    bool PropertyCheckbox(const char* label, bool& value, int indentLevel = 0);

    bool BeginPropertyCombo(const char* label, const char* preview_value, int indentLevel = 0);
    void EndPropertyCombo();
}