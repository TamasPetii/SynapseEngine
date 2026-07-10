#include "MaterialHierarchyView.h"
#include "Editor/Manager/EditorIcons.h"
#include "Editor/Widgets/CardWidget.h"
#include "Editor/Widgets/ItemCardWidget.h"
#include "Editor/Widgets/ItemCardContainerWidget.h"
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

                float currentY = ImGui::GetCursorScreenPos().y;
                float gridHeight = mainContentBottomY - currentY - 12.0f;
                if (gridHeight < 150.0f) gridHeight = 150.0f;

                ImGui::BeginChild("MaterialGridContainer", ImVec2(0, gridHeight), false, ImGuiWindowFlags_NoScrollbar);

                const auto& state = vm.GetState();
                const auto entries = state.filteredNodes;
                const float thumbnailSize = 100.0f;

                Syn::UI::ItemCardContainer("MaterialGrid", (int)entries.size(), thumbnailSize,
                    [&](int index) {
                        const auto& entry = entries[index];

                        Syn::UI::ItemCardDesc desc;
                        desc.id = entry.path.c_str();
                        desc.title = entry.name.c_str();

                        if (entry.hasPreview && state.atlasHandle) {
                            desc.texture = state.atlasHandle;
                            desc.uv0 = ImVec2(entry.uv0.x, entry.uv0.y);
                            desc.uv1 = ImVec2(entry.uv1.x, entry.uv1.y);
                        }
                        else {
                            desc.texture = InvalidTextureHandle;
                        }

                        desc.selected = (state.selectedMaterial == entry.id);

                        desc.events.onClick = [&vm, &entry] {
                            vm.Dispatch(MaterialSelectIntent{ entry.id });
                            };

                        desc.events.onDoubleClick = [&vm, &entry] {
                            };

                        desc.events.onDragDropSource = [&entry] {
                            };

                        Syn::UI::ItemCard(desc, thumbnailSize);
                    });

                ImGui::EndChild();
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
}