#include "HierarchyView.h"
#include "Editor/Manager/EditorIcons.h"
#include "Editor/Widgets/CardWidget.h"
#include <imgui.h>

namespace Syn {
    void HierarchyView::Draw(HierarchyViewModel& vm) {
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(8, 8));
        ImGuiWindowFlags windowFlags = ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse;

        if (ImGui::Begin(SYN_ICON_LIST " Scene Hierarchy", nullptr, windowFlags)) {

            auto getCardState = [this](const char* name) -> bool& {
                std::string key(name);
                if (_cardStates.find(key) == _cardStates.end()) _cardStates[key] = true;
                return _cardStates[key];
                };

            float mainContentBottomY = ImGui::GetWindowPos().y + ImGui::GetWindowContentRegionMax().y;
            constexpr const char* CardEntitiesTitle = "EntitiesCard";
            if (Syn::UI::BeginCard(CardEntitiesTitle, SYN_ICON_CUBE, getCardState(CardEntitiesTitle))) {

                RenderTopBar(vm);

                const auto& state = vm.GetState();

                ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.08f, 0.08f, 0.08f, 0.6f));
                ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, 8.0f);
                ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));

                float currentY = ImGui::GetCursorScreenPos().y;
                float tableHeight = mainContentBottomY - currentY - 12.0f;
                if (tableHeight < 100.0f) tableHeight = 100.0f;

                ImGui::BeginChild("HierarchyTableContainer", ImVec2(0, tableHeight), ImGuiChildFlags_Borders, ImGuiWindowFlags_NoScrollbar);

                if (ImGui::BeginTable("HierarchyTable", 2, ImGuiTableFlags_BordersInnerV | ImGuiTableFlags_ScrollY | ImGuiTableFlags_Resizable)) {

                    ImGui::TableSetupScrollFreeze(0, 1);
                    ImGui::TableSetupColumn("Name", ImGuiTableColumnFlags_WidthStretch);
                    ImGui::TableSetupColumn("Vis", ImGuiTableColumnFlags_WidthFixed, 32.0f);

                    ImGui::TableNextRow(ImGuiTableRowFlags_Headers);
                    for (int column = 0; column < 2; column++) {
                        ImGui::TableSetColumnIndex(column);
                        const char* columnName = ImGui::TableGetColumnName(column);

                        ImGui::PushID(column);

                        float cellWidth = ImGui::GetColumnWidth();
                        float textWidth = ImGui::CalcTextSize(columnName).x;
                        ImVec2 startPos = ImGui::GetCursorPos();

                        ImGui::TableHeader("");

                        ImGui::SetCursorPos(ImVec2(startPos.x + (cellWidth - textWidth) * 0.5f, startPos.y + 3.0f));
                        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 1.0f, 1.0f));
                        ImGui::Text("%s", columnName);
                        ImGui::PopStyleColor();

                        ImGui::PopID();
                    }

                    ImGuiListClipper clipper;
                    clipper.Begin(static_cast<int>(state.flatNodes.size()));

                    while (clipper.Step()) {
                        for (int row = clipper.DisplayStart; row < clipper.DisplayEnd; ++row) {
                            RenderEntityRow(vm, state.flatNodes[row]);
                        }
                    }

                    ImGui::EndTable();
                }
                ImGui::EndChild();
                ImGui::PopStyleVar(2);
                ImGui::PopStyleColor();

                if (ImGui::BeginDragDropTarget()) {
                    if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("ENTITY_DRAG")) {
                        EntityID droppedEntity = *(EntityID*)payload->Data;
                        if (droppedEntity != NULL_ENTITY) {
                            vm.Dispatch(HierarchyReparentEntityIntent{ droppedEntity, NULL_ENTITY });
                        }
                    }
                    ImGui::EndDragDropTarget();
                }

                if (ImGui::BeginPopupContextWindow("HierarchyContext", ImGuiPopupFlags_MouseButtonRight | ImGuiPopupFlags_NoOpenOverItems)) {
                    RenderContextMenu(vm, NULL_ENTITY);
                    ImGui::EndPopup();
                }

            }
            Syn::UI::EndCard();

            if (ImGui::IsMouseDown(ImGuiMouseButton_Left) && ImGui::IsWindowHovered()) {
                vm.Dispatch(HierarchySelectEntityIntent{ NULL_ENTITY });
            }
        }

        ImGui::End();
        ImGui::PopStyleVar();
    }

    void HierarchyView::RenderTopBar(HierarchyViewModel& vm) {
        ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(8, 6));

        float barHeight = ImGui::GetFrameHeight();
        ImGui::BeginChild("TopBar", ImVec2(0, barHeight), false, ImGuiWindowFlags_NoScrollbar);

        if (ImGui::Button(SYN_ICON_PLUS " Add")) {
            ImGui::OpenPopup("AddEntityPopup");
        }

        ImGui::SameLine();
        if (ImGui::Button(SYN_ICON_EXPAND_ALL)) {
            vm.Dispatch(HierarchyExpandAllIntent{});
        }
        if (ImGui::IsItemHovered()) ImGui::SetTooltip("Expand All");

        ImGui::SameLine();
        if (ImGui::Button(SYN_ICON_COLLAPSE_ALL)) {
            vm.Dispatch(HierarchyCollapseAllIntent{});
        }
        if (ImGui::IsItemHovered()) ImGui::SetTooltip("Collapse All");

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

        if (ImGui::InputTextWithHint("##SearchEntities", "Search...", searchBuffer, IM_ARRAYSIZE(searchBuffer))) {
            vm.Dispatch(HierarchySetSearchQueryIntent{ std::string(searchBuffer) });
        }

        if (ImGui::BeginPopup("AddEntityPopup")) {
            RenderContextMenu(vm, NULL_ENTITY);
            ImGui::EndPopup();
        }

        ImGui::EndChild();
        ImGui::PopStyleVar();

        ImGui::Spacing();
    }

    void HierarchyView::RenderEntityRow(HierarchyViewModel& vm, const HierarchyNode& node) {
        ImGui::PushID(node.id);
        ImGui::TableNextRow();

        ImGui::TableNextColumn();

        ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_SpanFullWidth | ImGuiTreeNodeFlags_FramePadding;

        if (!node.hasChildren) flags |= ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen;
        if (vm.GetState().selectedEntity == node.id) flags |= ImGuiTreeNodeFlags_Selected;

        ImGui::SetNextItemOpen(node.isExpanded, ImGuiCond_Always);

        float indentStep = 16.0f;
        ImGui::Indent(node.depth * indentStep);

        std::string label = node.icon + " " + node.name;
        bool isOpened = ImGui::TreeNodeEx((void*)(intptr_t)node.id, flags, "%s", label.c_str());

        ImGui::Unindent(node.depth * indentStep);

        if (ImGui::IsItemClicked(ImGuiMouseButton_Left) && !ImGui::IsItemToggledOpen()) {
            vm.Dispatch(HierarchySelectEntityIntent{ node.id });
        }

        if (ImGui::IsItemToggledOpen()) {
            vm.Dispatch(HierarchyToggleExpandIntent{ node.id, !node.isExpanded });
        }

        HandleDragAndDrop(vm, node.id);

        if (ImGui::BeginPopupContextItem()) {
            RenderContextMenu(vm, node.id);
            ImGui::EndPopup();
        }

        if (node.hasChildren && isOpened) {
            ImGui::TreePop();
        }

        ImGui::TableNextColumn();

        const char* eyeIcon = node.isVisible ? SYN_ICON_EYE : SYN_ICON_EYE_SLASH;
        ImVec4 eyeColor = node.isVisible ? ImGui::GetStyleColorVec4(ImGuiCol_Text) : ImVec4(0.5f, 0.5f, 0.5f, 1.0f);

        ImGui::PushStyleColor(ImGuiCol_Text, eyeColor);
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 0, 0, 0));

        float iconWidth = ImGui::CalcTextSize(eyeIcon).x;
        float columnWidth = ImGui::GetColumnWidth();
        ImGui::SetCursorPosX(ImGui::GetCursorPosX() + (columnWidth - iconWidth) * 0.5f - 4.0f);

        if (ImGui::Button(eyeIcon)) {
            vm.Dispatch(HierarchyToggleVisibilityIntent{ node.id, !node.isVisible });
        }

        ImGui::PopStyleColor(2);
        ImGui::PopID();
    }

    void HierarchyView::HandleDragAndDrop(HierarchyViewModel& vm, EntityID entity) {
        if (ImGui::BeginDragDropSource()) {
            ImGui::SetDragDropPayload("ENTITY_DRAG", &entity, sizeof(EntityID));
            ImGui::Text("Move Entity");
            ImGui::EndDragDropSource();
        }

        if (ImGui::BeginDragDropTarget()) {
            if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("ENTITY_DRAG")) {
                EntityID droppedEntity = *(EntityID*)payload->Data;
                if (droppedEntity != entity) {
                    vm.Dispatch(HierarchyReparentEntityIntent{ droppedEntity, entity });
                }
            }
            ImGui::EndDragDropTarget();
        }
    }

    void HierarchyView::RenderContextMenu(HierarchyViewModel& vm, EntityID contextEntity) {
        if (ImGui::MenuItem(SYN_ICON_CUBE " Empty Entity")) {
            vm.Dispatch(HierarchyCreateEntityIntent{ "Empty Entity", contextEntity });
        }

        ImGui::Separator();

        if (ImGui::MenuItem(SYN_ICON_VIDEO " Camera")) {
            vm.Dispatch(HierarchyCreateEntityIntent{ "Camera", contextEntity });
        }

        if (contextEntity != NULL_ENTITY) {
            ImGui::Separator();
            if (ImGui::MenuItem(SYN_ICON_TRASH " Delete")) {
                vm.Dispatch(HierarchyDestroyEntityIntent{ contextEntity });
            }
        }
    }
}