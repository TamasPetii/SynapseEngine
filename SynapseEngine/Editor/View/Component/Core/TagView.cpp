#include "TagView.h"
#include "Editor/Manager/EditorIcons.h"
#include "Editor/Widgets/CardWidget.h"
#include "Editor/Manager/EditorIcons.h"
#include <imgui.h>

namespace Syn {

    void TagView::Draw(TagViewModel& vm) {
        constexpr const char* CardTagTitle = "Tag & Identity";

        if (Syn::UI::BeginCard(CardTagTitle, SYN_ICON_TAG, _isCardOpen)) {

            TagState tagState = vm.GetState();

            ImGui::TextDisabled("Entity ID: %d", _entityId);
            ImGui::Spacing();

            bool isEnabled = tagState.isEnabled;
            if (ImGui::Checkbox("##EntityActive", &isEnabled)) {
                vm.Dispatch(ToggleEntityIntent{ isEnabled });
            }
            ImGui::SameLine();

            char nameBuffer[256];
            strncpy(nameBuffer, tagState.name.c_str(), sizeof(nameBuffer));
            nameBuffer[sizeof(nameBuffer) - 1] = '\0';
            ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
            if (ImGui::InputTextWithHint("##EntityName", "Entity Name", nameBuffer, IM_ARRAYSIZE(nameBuffer))) {
                vm.Dispatch(SetEntityNameIntent{ std::string(nameBuffer) });
            }

            ImGui::AlignTextToFramePadding();
            ImGui::Text("Tag");
            ImGui::SameLine(48.0f);

            char tagBuffer[256];
            strncpy(tagBuffer, tagState.tag.c_str(), sizeof(tagBuffer));
            tagBuffer[sizeof(tagBuffer) - 1] = '\0';
            ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
            if (ImGui::InputTextWithHint("##EntityTag", "Untagged", tagBuffer, IM_ARRAYSIZE(tagBuffer))) {
                vm.Dispatch(SetEntityTagIntent{ std::string(tagBuffer) });
            }
        }
        Syn::UI::EndCard();
    }
}