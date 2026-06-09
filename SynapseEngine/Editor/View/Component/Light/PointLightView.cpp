#include "PointLightView.h"
#include "Editor/Manager/EditorIcons.h"
#include "Editor/Widgets/CardWidget.h"
#include <imgui.h>

namespace Syn {

    void PointLightView::Draw(PointLightViewModel& vm) {
        PointLightState state = vm.GetState();

        if (!state.hasComponent) return;

        constexpr const char* CardTitle = "Point Light";

        if (Syn::UI::BeginCard(CardTitle, SYN_ICON_LIGHTBULB, _isCardOpen)) 
        {
            if (ImGui::ColorEdit3("Color", &state.color.x)) {
                vm.Dispatch(SetPointLightColorIntent{ state.color, !ImGui::IsItemDeactivatedAfterEdit() });
            }

            if (ImGui::DragFloat("Strength", &state.strength, 0.05f, 0.0f, 1000.0f, "%.2f")) {
                vm.Dispatch(SetPointLightStrengthIntent{ state.strength, !ImGui::IsItemDeactivatedAfterEdit() });
            }

            if (ImGui::DragFloat("Radius", &state.radius, 0.1f, 0.0f, 1000.0f, "%.2f")) {
                vm.Dispatch(SetPointLightRadiusIntent{ state.radius, !ImGui::IsItemDeactivatedAfterEdit() });
            }

            if (ImGui::DragFloat("Weaken Dist", &state.weakenDistance, 0.1f, 0.0f, 1000.0f, "%.2f")) {
                vm.Dispatch(SetPointLightWeakenIntent{ state.weakenDistance, !ImGui::IsItemDeactivatedAfterEdit() });
            }

            ImGui::Spacing();
            ImGui::Separator();
            ImGui::Spacing();

            bool useShadow = state.useShadow;
            if (ImGui::Checkbox("Cast Shadows", &useShadow)) {
                vm.Dispatch(SetPointLightUseShadowIntent{ useShadow });
            }

            if (useShadow) {
                ImGui::Indent(10.0f);

                if (ImGui::DragFloat("Near Plane", &state.shadowNearPlane, 0.01f, 0.01f, 100.0f, "%.3f")) {
                    vm.Dispatch(SetPointLightShadowNearIntent{ state.shadowNearPlane, !ImGui::IsItemDeactivatedAfterEdit() });
                }

                if (ImGui::DragFloat("Far Plane", &state.shadowFarPlane, 1.0f, 1.0f, 5000.0f, "%.1f")) {
                    vm.Dispatch(SetPointLightShadowFarIntent{ state.shadowFarPlane, !ImGui::IsItemDeactivatedAfterEdit() });
                }

                ImGui::Unindent(10.0f);
            }
        }
        Syn::UI::EndCard();
    }
}