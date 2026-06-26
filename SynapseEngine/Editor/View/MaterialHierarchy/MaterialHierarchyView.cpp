#include "MaterialHierarchyView.h"
#include "Editor/Manager/EditorIcons.h"
#include "Editor/Widgets/CardWidget.h"
#include <imgui.h>

namespace Syn {
    void MaterialHierarchyView::Draw(MaterialHierarchyViewModel& vm) {
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(8, 8));
        ImGuiWindowFlags windowFlags = ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse;

        if (ImGui::Begin(SYN_ICON_BRUSH " Materials", nullptr, windowFlags)) {

            auto getCardState = [this](const char* name) -> bool& {
                std::string key(name);
                if (_cardStates.find(key) == _cardStates.end()) _cardStates[key] = true;
                return _cardStates[key];
                };

            float mainContentBottomY = ImGui::GetWindowPos().y + ImGui::GetWindowContentRegionMax().y;
            constexpr const char* CardMaterialsTitle = "MaterialListCard";

            if (Syn::UI::BeginCard(CardMaterialsTitle, SYN_ICON_BRUSH, getCardState(CardMaterialsTitle))) {

                RenderTopBar(vm);

                const auto& state = vm.GetState();

                ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.08f, 0.08f, 0.08f, 0.6f));
                ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, 8.0f);
                ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));

                float currentY = ImGui::GetCursorScreenPos().y;
                float tableHeight = mainContentBottomY - currentY - 12.0f;
                if (tableHeight < 100.0f) tableHeight = 100.0f;

                ImGui::BeginChild("MaterialTableContainer", ImVec2(0, tableHeight), ImGuiChildFlags_Borders, ImGuiWindowFlags_NoScrollbar);

                if (ImGui::BeginTable("MaterialTable", 1, ImGuiTableFlags_BordersInnerV | ImGuiTableFlags_ScrollY | ImGuiTableFlags_Resizable)) {
                    ImGui::TableSetupScrollFreeze(0, 1);
                    ImGui::TableSetupColumn("Name", ImGuiTableColumnFlags_WidthStretch);

                    ImGui::TableNextRow(ImGuiTableRowFlags_Headers);
                    ImGui::TableSetColumnIndex(0);

                    float cellWidth = ImGui::GetColumnWidth();
                    float textWidth = ImGui::CalcTextSize("Name").x;
                    ImVec2 startPos = ImGui::GetCursorPos();

                    ImGui::TableHeader("");
                    ImGui::SetCursorPos(ImVec2(startPos.x + (cellWidth - textWidth) * 0.5f, startPos.y + 3.0f));
                    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 1.0f, 1.0f));
                    ImGui::Text("Name");
                    ImGui::PopStyleColor();

                    ImGuiListClipper clipper;
                    clipper.Begin(static_cast<int>(state.filteredNodes.size()));

                    while (clipper.Step()) {
                        for (int row = clipper.DisplayStart; row < clipper.DisplayEnd; ++row) {
                            RenderMaterialRow(vm, state.filteredNodes[row]);
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
                vm.Dispatch(MaterialSelectIntent{ INVALID_MATERIAL_ID });
            }
        }
        ImGui::End();
        ImGui::PopStyleVar();
    }

    void MaterialHierarchyView::RenderTopBar(MaterialHierarchyViewModel& vm) {
        ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(8, 6));

        float barHeight = ImGui::GetFrameHeight();
        ImGui::BeginChild("MaterialTopBar", ImVec2(0, barHeight), false, ImGuiWindowFlags_NoScrollbar);

        if (ImGui::Button(SYN_ICON_SYNC " Refresh")) {
            vm.Dispatch(MaterialRefreshIntent{});
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

        if (ImGui::InputTextWithHint("##SearchMaterials", "Search...", searchBuffer, IM_ARRAYSIZE(searchBuffer))) {
            vm.Dispatch(MaterialSetSearchQueryIntent{ std::string(searchBuffer) });
        }

        ImGui::EndChild();
        ImGui::PopStyleVar();
        ImGui::Spacing();
    }

    void MaterialHierarchyView::RenderMaterialRow(MaterialHierarchyViewModel& vm, const MaterialNode& node) {
        ImGui::PushID(node.id);
        ImGui::TableNextRow();
        ImGui::TableNextColumn();

        ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen | ImGuiTreeNodeFlags_SpanFullWidth | ImGuiTreeNodeFlags_FramePadding;
        if (vm.GetState().selectedMaterial == node.id) {
            flags |= ImGuiTreeNodeFlags_Selected;
        }

        std::string label = node.icon + " " + node.name;
        ImGui::TreeNodeEx((void*)(intptr_t)node.id, flags, "%s", label.c_str());

        if (ImGui::IsItemClicked(ImGuiMouseButton_Left)) {
            vm.Dispatch(MaterialSelectIntent{ node.id });
        }

        ImGui::PopID();
    }
}