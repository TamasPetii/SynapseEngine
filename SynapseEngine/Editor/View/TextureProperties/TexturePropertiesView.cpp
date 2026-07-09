#include "TexturePropertiesView.h"
#include "Editor/Manager/EditorIcons.h"
#include "Editor/Widgets/CardWidget.h"
#include "Editor/Widgets/PropertyGrid.h"
#include <imgui.h>
#include <string>

namespace Syn {
    void TexturePropertiesView::Draw(TexturePropertiesViewModel& vm) {
        const TexturePropertiesState& state = vm.GetState();

        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(8, 8));
        ImGuiWindowFlags windowFlags = ImGuiWindowFlags_None;

        if (ImGui::Begin(SYN_ICON_INFO_CIRCLE " Texture Properties", nullptr, windowFlags)) {

            auto getCardState = [this](const char* name) -> bool& {
                std::string key(name);
                if (_cardStates.find(key) == _cardStates.end()) _cardStates[key] = true;
                return _cardStates[key];
                };

            constexpr const char* CardTitle = "TextureDetailsCard";
            if (Syn::UI::BeginCard(CardTitle, SYN_ICON_IMAGE, getCardState(CardTitle))) {

                if (!state.hasSelection) {
                    ImGui::TextDisabled("No texture selected.");
                }
                else {
                    if (Syn::UI::BeginPropertyGrid("TexturePropsGrid"))
                    {
                        std::string dimensions = std::to_string(state.width) + " x " + std::to_string(state.height);
                        if (state.depth > 1) {
                            dimensions += " x " + std::to_string(state.depth);
                        }
                        DrawReadOnlyProperty("Dimensions", dimensions.c_str());
                        DrawReadOnlyProperty("Mip Levels", std::to_string(state.mipLevels).c_str());

                        std::string formatStr = "VkFormat(" + std::to_string(state.format) + ")";
                        DrawReadOnlyProperty("Format", formatStr.c_str());
                        DrawReadOnlyProperty("Compressed", state.isCompressed ? "Yes" : "No");

                        Syn::UI::EndPropertyGrid();
                    }
                }
            }
            Syn::UI::EndCard();
        }

        ImGui::End();
        ImGui::PopStyleVar();
    }

    void TexturePropertiesView::DrawReadOnlyProperty(const char* label, const char* value) {
        ImGui::TableNextRow();

        ImGui::TableNextColumn();
        ImGui::TextUnformatted(label);

        ImGui::TableNextColumn();
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.7f, 0.7f, 0.7f, 1.0f));
        ImGui::TextUnformatted(value);
        ImGui::PopStyleColor();
    }
}