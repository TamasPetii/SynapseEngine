#include "ComponentView.h"
#include "Editor/Manager/EditorIcons.h"
#include "Editor/Widgets/CardWidget.h"
#include "Editor/Widgets/Vector3Widget.h"
#include <imgui.h>

namespace Syn {

    void ComponentView::Draw(ComponentViewModel& vm) {
        const ComponentState& state = vm.GetState();

        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(8, 8));

        if (ImGui::Begin(SYN_ICON_INFO_CIRCLE " Inspector")) {

            if (!state.hasSelection) {
                ImGui::TextDisabled("No entity selected.");
                ImGui::End();
                ImGui::PopStyleVar();
                return;
            }

            auto getCardState = [this](const char* name) -> bool& {
                std::string key(name);
                if (_cardStates.find(key) == _cardStates.end()) _cardStates[key] = true;
                return _cardStates[key];
                };

            constexpr const char* CardTagTitle = "Tag & Identity";
            if (Syn::UI::BeginCard(CardTagTitle, SYN_ICON_TAG, getCardState(CardTagTitle))) {

                TagViewModel& tagVM = vm.GetTagVM();
                TagState tagState = tagVM.GetState();

                ImGui::TextDisabled("Entity ID: %d", state.activeEntityId);
                ImGui::Spacing();

                bool isEnabled = tagState.isEnabled;
                if (ImGui::Checkbox("##EntityActive", &isEnabled)) {
                    tagVM.Dispatch(ToggleEntityIntent{ isEnabled });
                }
                ImGui::SameLine();

                char nameBuffer[256];
                strncpy(nameBuffer, tagState.name.c_str(), sizeof(nameBuffer));
                nameBuffer[sizeof(nameBuffer) - 1] = '\0';
                ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
                if (ImGui::InputTextWithHint("##EntityName", "Entity Name", nameBuffer, IM_ARRAYSIZE(nameBuffer))) {
                    tagVM.Dispatch(SetEntityNameIntent{ std::string(nameBuffer) });
                }

                ImGui::AlignTextToFramePadding();
                ImGui::Text("Tag");
                ImGui::SameLine(48.0f);

                char tagBuffer[256];
                strncpy(tagBuffer, tagState.tag.c_str(), sizeof(tagBuffer));
                tagBuffer[sizeof(tagBuffer) - 1] = '\0';
                ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
                if (ImGui::InputTextWithHint("##EntityTag", "Untagged", tagBuffer, IM_ARRAYSIZE(tagBuffer))) {
                    tagVM.Dispatch(SetEntityTagIntent{ std::string(tagBuffer) });
                }
            }
            Syn::UI::EndCard();


            constexpr const char* CardTransformTitle = "Transform";
            if (Syn::UI::BeginCard(CardTransformTitle, SYN_ICON_ARROWS_ALT, getCardState(CardTransformTitle))) {

                TransformViewModel& tVM = vm.GetTransformVM();
                TransformState tState = tVM.GetState();

                bool changed = false;
                bool deactivated = false;

                changed = Syn::UI::DrawVec3Control("Position", tState.position, 0.0f, deactivated);
                if (changed || deactivated) {
                    tVM.Dispatch(SetPositionIntent{ tState.position, !deactivated });
                }

                changed = Syn::UI::DrawVec3Control("Rotation", tState.rotation, 0.0f, deactivated);
                if (changed || deactivated) {
                    tVM.Dispatch(SetRotationIntent{ tState.rotation, !deactivated });
                }

                changed = Syn::UI::DrawVec3Control("Scale", tState.scale, 1.0f, deactivated);
                if (changed || deactivated) {
                    tVM.Dispatch(SetScaleIntent{ tState.scale, !deactivated });
                }
            }
            Syn::UI::EndCard();

            ImGui::Spacing();
            ImGui::Separator();
            ImGui::Spacing();

            ImGui::SetCursorPosX((ImGui::GetWindowSize().x - 150.0f) * 0.5f);
            if (ImGui::Button(SYN_ICON_PLUS " Add Component", ImVec2(150.0f, 32.0f))) {
                ImGui::OpenPopup("AddComponentPopup");
            }

            if (ImGui::BeginPopup("AddComponentPopup")) {
                ImGui::TextDisabled("Available Components");
                ImGui::Separator();

                if (ImGui::MenuItem(SYN_ICON_LIGHTBULB " Point Light")) {
                    // TODO: Dispatch add component intent
                }
                if (ImGui::MenuItem(SYN_ICON_CUBE " Mesh Renderer")) {
                    // TODO: Dispatch add component intent
                }

                ImGui::EndPopup();
            }
        }

        ImGui::End();
        ImGui::PopStyleVar();
    }
}