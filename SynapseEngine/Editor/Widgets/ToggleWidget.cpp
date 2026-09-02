// Copyright (C) 2026 Tamás Péter
// This file is part of SynapseEngine.
//
// SynapseEngine is free software: you can redistribute it and/or modify
// it under the terms of the GNU Affero General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// SynapseEngine is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License
// along with SynapseEngine. If not, see <https://www.gnu.org/licenses/>.

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