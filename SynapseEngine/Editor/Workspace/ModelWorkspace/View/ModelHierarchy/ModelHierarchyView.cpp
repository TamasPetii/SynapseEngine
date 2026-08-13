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

#include "ModelHierarchyView.h"
#include "Editor/Manager/EditorIcons.h"
#include "Editor/Widgets/CardWidget.h"
#include "Editor/Widgets/ItemCardWidget.h"
#include "Editor/Widgets/ItemCardContainerWidget.h"
#include <imgui.h>

namespace Syn
{
    void ModelHierarchyView::Draw(ModelHierarchyViewModel& vm) {
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(8, 8));
        ImGuiWindowFlags windowFlags = ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse;

        const auto& state = vm.GetState();

        auto getCardState = [this](const char* name) -> bool& {
            std::string key(name);
            if (_cardStates.find(key) == _cardStates.end()) _cardStates[key] = true;
            return _cardStates[key];
            };

        if (ImGui::Begin(SYN_ICON_CUBE " Models", nullptr, windowFlags)) {

            float mainContentBottomY = ImGui::GetWindowPos().y + ImGui::GetWindowContentRegionMax().y;
            constexpr const char* CardModelTitle = "ModelListCard";

            if (Syn::UI::BeginCard(CardModelTitle, SYN_ICON_CUBE, getCardState(CardModelTitle))) {
                RenderTopBar(vm);

                float currentY = ImGui::GetCursorScreenPos().y;
                float gridHeight = mainContentBottomY - currentY - 12.0f;
                if (gridHeight < 150.0f) gridHeight = 150.0f;

                ImGui::BeginChild("ModelGridContainer", ImVec2(0, gridHeight), false, ImGuiWindowFlags_NoScrollbar);

                const auto& entries = state.filteredModels;
                const float thumbnailSize = 100.0f;

                Syn::UI::ItemCardContainer("ModelGrid", (int)entries.size(), thumbnailSize,
                    [&](int index) {
                        const auto& entry = entries[index];

                        Syn::UI::ItemCardDesc desc;
                        std::string idStr = std::to_string(entry.id);
                        desc.id = idStr.c_str();
                        desc.title = entry.name.c_str();

                        if (entry.hasPreview && state.atlasHandle) {
                            desc.texture = state.atlasHandle;
                            desc.uv0 = ImVec2(entry.uv0.x, entry.uv0.y);
                            desc.uv1 = ImVec2(entry.uv1.x, entry.uv1.y);
                        }
                        else {
                            desc.texture = InvalidTextureHandle;
                        }

                        desc.selected = (state.selectedModelId == entry.id);

                        desc.events.onClick = [&vm, &entry] {
                            vm.Dispatch(ModelHierarchySelectIntent{ entry.id, -1 });
                            };

                        desc.events.onDoubleClick = [&vm, &entry] {};
                        desc.events.onDragDropSource = [&entry] {};

                        Syn::UI::ItemCard(desc, thumbnailSize);
                    });

                ImGui::EndChild();
            }
            Syn::UI::EndCard();

            if (ImGui::IsMouseDown(ImGuiMouseButton_Left) && ImGui::IsWindowHovered()) {
                vm.Dispatch(ModelHierarchySelectIntent{ 0xFFFFFFFF, -1 });
            }
        }
        ImGui::End();


        if (ImGui::Begin(SYN_ICON_LIST " Internal Hierarchy", nullptr, windowFlags)) {

            float mainContentBottomY = ImGui::GetWindowPos().y + ImGui::GetWindowContentRegionMax().y;
            constexpr const char* CardHierarchyTitle = "Internal Hierarchy Card";

            if (Syn::UI::BeginCard(CardHierarchyTitle, SYN_ICON_LIST, getCardState(CardHierarchyTitle))) {

                if (state.selectedModelId != 0xFFFFFFFF) {
                    ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.08f, 0.08f, 0.08f, 0.6f));
                    ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, 8.0f);
                    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));

                    float currentY = ImGui::GetCursorScreenPos().y;
                    float tableHeight = mainContentBottomY - currentY - 12.0f;
                    if (tableHeight < 100.0f) tableHeight = 100.0f;

                    ImGui::BeginChild("ModelHierarchyTableContainer", ImVec2(0, tableHeight), ImGuiChildFlags_Borders, ImGuiWindowFlags_NoScrollbar);

                    if (ImGui::BeginTable("ModelTable", 1, ImGuiTableFlags_ScrollY | ImGuiTableFlags_Resizable)) {
                        ImGui::TableSetupScrollFreeze(0, 1);
                        ImGui::TableSetupColumn("Hierarchy", ImGuiTableColumnFlags_WidthStretch);

                        ImGui::TableNextRow(ImGuiTableRowFlags_Headers);
                        ImGui::TableSetColumnIndex(0);

                        float cellWidth = ImGui::GetColumnWidth();
                        float textWidth = ImGui::CalcTextSize("Hierarchy").x;
                        ImVec2 startPos = ImGui::GetCursorPos();

                        ImGui::TableHeader("##ColHierarchy");
                        ImGui::SetCursorPos(ImVec2(startPos.x + (cellWidth - textWidth) * 0.5f, startPos.y + 3.0f));
                        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 1.0f, 1.0f));
                        ImGui::Text("Hierarchy");
                        ImGui::PopStyleColor();

                        ImGuiListClipper clipper;
                        clipper.Begin(static_cast<int>(state.flatNodes.size()));

                        while (clipper.Step()) {
                            for (int row = clipper.DisplayStart; row < clipper.DisplayEnd; ++row) {
                                RenderNodeRow(vm, state.flatNodes[row]);
                            }
                        }

                        ImGui::EndTable();
                    }
                    ImGui::EndChild();
                    ImGui::PopStyleVar(2);
                    ImGui::PopStyleColor();
                }
                else {
                    ImGui::Spacing();
                    ImGui::Indent(8.0f);
                    ImGui::TextDisabled("Select a model to view its internal hierarchy.");
                    ImGui::Unindent(8.0f);
                }
            }
            Syn::UI::EndCard();
        }
        ImGui::End();

        ImGui::PopStyleVar();
    }

    void ModelHierarchyView::RenderTopBar(ModelHierarchyViewModel& vm) {
        ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(8, 6));

        float barHeight = ImGui::GetFrameHeight();
        ImGui::BeginChild("ModelTopBar", ImVec2(0, barHeight), false, ImGuiWindowFlags_NoScrollbar);

        if (ImGui::Button(SYN_ICON_SYNC " Refresh##ModelHierarchy")) {
            vm.Dispatch(ModelHierarchyRefreshIntent{});
        }

        ImGui::SameLine();
        ImGui::Dummy(ImVec2(8.0f, 0.0f));
        ImGui::SameLine();

        ImGui::AlignTextToFramePadding();
        ImGui::TextDisabled(SYN_ICON_SEARCH);
        ImGui::SameLine();

        ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);

        const auto& state = vm.GetState();
        char searchBuffer[256];
        strncpy(searchBuffer, state.searchQuery.c_str(), sizeof(searchBuffer));
        searchBuffer[sizeof(searchBuffer) - 1] = '\0';

        if (ImGui::InputTextWithHint("##SearchModels", "Search...", searchBuffer, IM_ARRAYSIZE(searchBuffer))) {
            vm.Dispatch(ModelHierarchySetSearchIntent{ std::string(searchBuffer) });
        }

        ImGui::EndChild();
        ImGui::PopStyleVar();
        ImGui::Spacing();
    }

    void ModelHierarchyView::RenderNodeRow(ModelHierarchyViewModel& vm, const ModelHierarchyNode& node) {
        ImGui::TableNextRow();
        ImGui::TableNextColumn();

        ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_SpanFullWidth | ImGuiTreeNodeFlags_FramePadding | ImGuiTreeNodeFlags_NoTreePushOnOpen;

        if (!node.hasChildren) {
            flags |= ImGuiTreeNodeFlags_Leaf;
        }

        const auto& state = vm.GetState();
        if (state.selectedModelId == node.modelId && state.selectedDescriptorIndex == node.descriptorIndex) {
            flags |= ImGuiTreeNodeFlags_Selected;
        }

        ImGui::SetNextItemOpen(node.isExpanded, ImGuiCond_Always);

        float indentStep = 16.0f;
        ImGui::Indent(node.depth * indentStep);

        std::string label = node.icon + " " + node.name + "##NodeID_" + std::to_string(node.modelId) + "_" + std::to_string(node.descriptorIndex);

        ImGui::TreeNodeEx(label.c_str(), flags);

        ImGui::Unindent(node.depth * indentStep);

        if (ImGui::IsItemClicked(ImGuiMouseButton_Left) && !ImGui::IsItemToggledOpen()) {
            vm.Dispatch(ModelHierarchySelectIntent{ node.modelId, node.descriptorIndex });
        }

        if (ImGui::IsItemToggledOpen()) {
            vm.Dispatch(ModelHierarchyToggleExpandIntent{ node.modelId, node.descriptorIndex, !node.isExpanded });
        }
    }
}