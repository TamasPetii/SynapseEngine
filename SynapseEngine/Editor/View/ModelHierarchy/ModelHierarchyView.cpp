#include "ModelHierarchyView.h"
#include "Editor/Manager/EditorIcons.h"
#include "Editor/Widgets/CardWidget.h"
#include <imgui.h>

namespace Syn 
{
    void ModelHierarchyView::Draw(ModelHierarchyViewModel& vm) {
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(8, 8));
        ImGuiWindowFlags windowFlags = ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse;

        if (ImGui::Begin(SYN_ICON_CUBE " Model Hierarchy", nullptr, windowFlags)) {

            auto getCardState = [this](const char* name) -> bool& {
                std::string key(name);
                if (_cardStates.find(key) == _cardStates.end()) _cardStates[key] = true;
                return _cardStates[key];
                };

            float mainContentBottomY = ImGui::GetWindowPos().y + ImGui::GetWindowContentRegionMax().y;
            constexpr const char* CardModelTitle = "ModelListCard";

            if (Syn::UI::BeginCard(CardModelTitle, SYN_ICON_CUBE, getCardState(CardModelTitle))) {

                RenderTopBar(vm);

                const auto& state = vm.GetState();

                ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.08f, 0.08f, 0.08f, 0.6f));
                ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, 8.0f);
                ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));

                float currentY = ImGui::GetCursorScreenPos().y;
                float tableHeight = mainContentBottomY - currentY - 12.0f;
                if (tableHeight < 100.0f) tableHeight = 100.0f;

                ImGui::BeginChild("ModelHierarchyTableContainer", ImVec2(0, tableHeight), ImGuiChildFlags_Borders, ImGuiWindowFlags_NoScrollbar);

                if (ImGui::BeginTable("ModelTable", 2, ImGuiTableFlags_BordersInnerV | ImGuiTableFlags_ScrollY | ImGuiTableFlags_Resizable)) {
                    ImGui::TableSetupScrollFreeze(0, 1);
                    ImGui::TableSetupColumn("Hierarchy", ImGuiTableColumnFlags_WidthStretch);
                    ImGui::TableSetupColumn("Tris", ImGuiTableColumnFlags_WidthFixed, 45.0f);

                    ImGui::TableNextRow(ImGuiTableRowFlags_Headers);

                    ImGui::TableSetColumnIndex(0);
                    float cellWidth = ImGui::GetColumnWidth();
                    float textWidth = ImGui::CalcTextSize("Hierarchy").x;
                    ImVec2 startPos = ImGui::GetCursorPos();

                    ImGui::TableHeader("##ColHierarchy");

                    ImGui::SetCursorPos(ImVec2(startPos.x + (cellWidth - textWidth) * 0.5f, startPos.y + 3.0f));
                    ImGui::Text("Hierarchy");

                    ImGui::TableSetColumnIndex(1);
                    cellWidth = ImGui::GetColumnWidth();
                    textWidth = ImGui::CalcTextSize("Tris").x;
                    startPos = ImGui::GetCursorPos();

                    ImGui::TableHeader("##ColTris");

                    ImGui::SetCursorPos(ImVec2(startPos.x + (cellWidth - textWidth) * 0.5f, startPos.y + 3.0f));
                    ImGui::Text("Tris");

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
            Syn::UI::EndCard();

            if (ImGui::IsMouseDown(ImGuiMouseButton_Left) && ImGui::IsWindowHovered()) {
                vm.Dispatch(ModelHierarchySelectIntent{ INVALID_MODEL_ID, -1 });
            }
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

        ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_SpanFullWidth | ImGuiTreeNodeFlags_FramePadding | ImGuiTreeNodeFlags_NoTreePushOnOpen;

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

        ImGui::TableNextColumn();

        if (node.triangleCount > 0) {
            ImGui::TextDisabled("%u", node.triangleCount);
        }
    }
}