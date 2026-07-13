#include "TagView.h"
#include "Editor/Manager/EditorIcons.h"
#include "Editor/Widgets/CardWidget.h"
#include "Editor/Manager/EditorIcons.h"
#include "Editor/Widgets/PropertyGrid.h"
#include <imgui.h>

namespace Syn {

    void TagView::Draw(TagViewModel& vm) {
        constexpr const char* CardTagTitle = "Tag & Identity";

        if (Syn::UI::BeginCard(CardTagTitle, SYN_ICON_TAG, _isCardOpen)) {

            TagState tagState = vm.GetState();

            if (Syn::UI::BeginPropertyGrid("TagPropsGrid")) {

                Syn::UI::BeginProperty("Entity ID");
                ImGui::TextDisabled("%d", _entityId);

                bool isEnabled = tagState.isEnabled;
                if (Syn::UI::PropertyCheckbox("Is Active", isEnabled)) {
                    vm.Dispatch(ToggleEntityIntent{ isEnabled });
                }

                Syn::UI::BeginProperty("Name");
                char nameBuffer[256];
                strncpy(nameBuffer, tagState.name.c_str(), sizeof(nameBuffer));
                nameBuffer[sizeof(nameBuffer) - 1] = '\0';
                if (ImGui::InputTextWithHint("##EntityName", "Entity Name", nameBuffer, IM_ARRAYSIZE(nameBuffer))) {
                    vm.Dispatch(SetEntityNameIntent{ std::string(nameBuffer) });
                }

                Syn::UI::BeginProperty("Tag");
                char tagBuffer[256];
                strncpy(tagBuffer, tagState.tag.c_str(), sizeof(tagBuffer));
                tagBuffer[sizeof(tagBuffer) - 1] = '\0';
                if (ImGui::InputTextWithHint("##EntityTag", "Untagged", tagBuffer, IM_ARRAYSIZE(tagBuffer))) {
                    vm.Dispatch(SetEntityTagIntent{ std::string(tagBuffer) });
                }

                Syn::UI::EndPropertyGrid();
            }
        }

        Syn::UI::EndCard();
    }
}