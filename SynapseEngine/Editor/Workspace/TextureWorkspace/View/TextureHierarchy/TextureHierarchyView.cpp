#include "TextureHierarchyView.h"
#include "Editor/Manager/EditorIcons.h"
#include "Editor/Widgets/CardWidget.h"
#include "Editor/Widgets/ItemCardWidget.h"
#include "Editor/Widgets/ItemCardContainerWidget.h"
#include <imgui.h>

namespace Syn {
    void TextureHierarchyView::Draw(TextureHierarchyViewModel& vm) {
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(8, 8));
        ImGuiWindowFlags windowFlags = ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse;

        if (ImGui::Begin(SYN_ICON_IMAGE " Textures", nullptr, windowFlags)) {

            auto getCardState = [this](const char* name) -> bool& {
                std::string key(name);
                if (_cardStates.find(key) == _cardStates.end()) _cardStates[key] = true;
                return _cardStates[key];
                };

            float mainContentBottomY = ImGui::GetWindowPos().y + ImGui::GetWindowContentRegionMax().y;
            constexpr const char* CardTexturesTitle = "TextureListCard";

            if (Syn::UI::BeginCard(CardTexturesTitle, SYN_ICON_IMAGE, getCardState(CardTexturesTitle))) {

                RenderTopBar(vm);

                float currentY = ImGui::GetCursorScreenPos().y;
                float gridHeight = mainContentBottomY - currentY - 12.0f;
                if (gridHeight < 150.0f) gridHeight = 150.0f;

                ImGui::BeginChild("TextureGridContainer", ImVec2(0, gridHeight), false, ImGuiWindowFlags_NoScrollbar);

                const auto& state = vm.GetState();

                const auto entries = state.filteredNodes;
                const float thumbnailSize = 100;
                Syn::UI::ItemCardContainer("ContentGrid", (int)entries.size(), thumbnailSize,
                    [&](int index) {
                        const auto& entry = entries[index];

                        Syn::UI::ItemCardDesc desc;
                        desc.id = entry.path.c_str();
                        desc.title = entry.name.c_str();
                        desc.texture = entry.handle;
                        desc.selected = (state.selectedTexture == entry.id);

                        desc.events.onClick = [&vm, &entry] {
                            vm.Dispatch(TextureSelectIntent{ entry.id });
                            };

                        desc.events.onDoubleClick = [&vm, &entry] {
                            //Todo?
                            };

                        desc.events.onDragDropSource = [this, &entry] {
                            //Todo?
                            };

                        Syn::UI::ItemCard(desc, thumbnailSize);
                    });

                ImGui::EndChild();
            }
            Syn::UI::EndCard();

            if (ImGui::IsMouseDown(ImGuiMouseButton_Left) && ImGui::IsWindowHovered()) {
                vm.Dispatch(TextureSelectIntent{ INVALID_TEXTURE_ID });
            }
        }
        ImGui::End();
        ImGui::PopStyleVar();
    }

    void TextureHierarchyView::RenderTopBar(TextureHierarchyViewModel& vm) {
        ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(8, 6));

        float barHeight = ImGui::GetFrameHeight();
        ImGui::BeginChild("TextureTopBar", ImVec2(0, barHeight), false, ImGuiWindowFlags_NoScrollbar);

        if (ImGui::Button(SYN_ICON_SYNC " Refresh")) {
            vm.Dispatch(TextureRefreshIntent{});
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

        if (ImGui::InputTextWithHint("##SearchTextures", "Search...", searchBuffer, IM_ARRAYSIZE(searchBuffer))) {
            vm.Dispatch(TextureSetSearchQueryIntent{ std::string(searchBuffer) });
        }

        ImGui::EndChild();
        ImGui::PopStyleVar();
        ImGui::Spacing();
    }
}