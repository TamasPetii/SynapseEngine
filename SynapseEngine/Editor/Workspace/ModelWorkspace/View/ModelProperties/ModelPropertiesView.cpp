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

#include "ModelPropertiesView.h"
#include "Editor/Manager/EditorIcons.h"
#include "Editor/Widgets/CardWidget.h"
#include "Editor/Widgets/PropertyGrid.h"
#include <imgui.h>

namespace Syn {
    void ModelPropertiesView::Draw(ModelPropertiesViewModel& vm) {
        const auto& state = vm.GetState();

        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(8, 8));

        if (ImGui::Begin(SYN_ICON_INFO_CIRCLE " Model Properties##ModelPropsWindow", nullptr)) {

            if (!state.hasSelection) {
                ImGui::TextDisabled("No model or mesh selected.");
                ImGui::End();
                ImGui::PopStyleVar();
                return;
            }

            auto getCardState = [this](const char* name) -> bool& {
                std::string key(name);
                if (_cardStates.find(key) == _cardStates.end()) _cardStates[key] = true;
                return _cardStates[key];
                };

            if (Syn::UI::BeginCard("ModelGlobalPropsCard", SYN_ICON_CUBE, getCardState("ModelGlobalPropsCard"))) {
                ImGui::Text("Model: "); ImGui::SameLine();
                ImGui::TextColored(ImVec4(0.4f, 0.8f, 1.0f, 1.0f), "%s", state.modelName.c_str());

                if (Syn::UI::BeginPropertyGrid("GlobalGrid")) {
                    DrawPropertyRow("Vertices", std::to_string(state.globalVertexCount));
                    DrawPropertyRow("Indices", std::to_string(state.globalIndexCount));
                    DrawPropertyRow("Triangles", std::to_string(state.globalIndexCount / 3));
                    Syn::UI::EndPropertyGrid();
                }  
            }
            Syn::UI::EndCard();

            if (state.isNodeSelected) {
                ImGui::Spacing();
                if (Syn::UI::BeginCard("NodePropsCard", SYN_ICON_PROJECT_DIAGRAM, getCardState("NodePropsCard"))) {
                    ImGui::Text("Node: "); ImGui::SameLine();
                    ImGui::TextColored(ImVec4(0.4f, 1.0f, 0.6f, 1.0f), "%s", state.nodeName.c_str());

                    if (Syn::UI::BeginPropertyGrid("NodeGrid")) {
                        DrawPropertyRow("Mesh Index", state.meshIndex == 0xFFFF ? "None" : std::to_string(state.meshIndex));
                        DrawPropertyRow("Triangles", std::to_string(state.nodeIndexCount / 3));
                        DrawPropertyRow("Meshlets", std::to_string(state.nodeMeshletCount));
                        Syn::UI::EndPropertyGrid();
                    }
                }
                Syn::UI::EndCard();
            }
        }

        ImGui::End();
        ImGui::PopStyleVar();
    }

    void ModelPropertiesView::DrawPropertyRow(const char* label, const std::string& value) {
        Syn::UI::PropertyText(label, value.c_str());
    }
}