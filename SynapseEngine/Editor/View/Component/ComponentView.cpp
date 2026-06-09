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

            //Tag
            _tagView.SetActiveEntity(state.activeEntityId);
            _tagView.Draw(vm.GetTagViewModel());

            //Transform
            _transformView.Draw(vm.GetTransformViewModel());

            //DirectionLight
            _directionLightView.Draw(vm.GetDirectionLightViewModel());

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