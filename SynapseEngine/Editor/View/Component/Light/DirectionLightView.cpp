#include "DirectionLightView.h"
#include "Editor/Manager/EditorIcons.h"
#include "Editor/Widgets/CardWidget.h"
#include <imgui.h>

namespace Syn {

    void DirectionLightView::Draw(DirectionLightViewModel& vm) {
        DirectionLightState state = vm.GetState();

        if (!state.hasComponent) return;

        constexpr const char* CardTitle = "Directional Light";
        if (Syn::UI::BeginCard(CardTitle, SYN_ICON_SUN, _isCardOpen))
        {
            bool isDeactivated = false;

            if (ImGui::ColorEdit3("Color", &state.color.x)) {
                vm.Dispatch(SetLightColorIntent{ state.color, !ImGui::IsItemDeactivatedAfterEdit() });
            }

            if (ImGui::DragFloat("Strength", &state.strength, 0.05f, 0.0f, 1000.0f, "%.2f")) {
                vm.Dispatch(SetLightStrengthIntent{ state.strength, !ImGui::IsItemDeactivatedAfterEdit() });
            }

            ImGui::Spacing();
            ImGui::Separator();
            ImGui::Spacing();

            bool useShadow = state.useShadow;
            if (ImGui::Checkbox("Cast Shadows", &useShadow)) {
                vm.Dispatch(SetLightUseShadowIntent{ useShadow });
            }

            if (useShadow) {
                ImGui::Indent(10.0f);

                if (ImGui::DragFloat("Shadow Distance", &state.shadowFarPlane, 1.0f, 10.0f, 5000.0f, "%.1f")) {
                    vm.Dispatch(SetShadowFarPlaneIntent{ state.shadowFarPlane, !ImGui::IsItemDeactivatedAfterEdit() });
                }

                if (ImGui::DragFloat4("Cascade Splits", &state.cascadeSplits.x, 0.01f, 0.0f, 1.0f, "%.3f")) {
                    vm.Dispatch(SetCascadeSplitsIntent{ state.cascadeSplits, !ImGui::IsItemDeactivatedAfterEdit() });
                }

                ImGui::Unindent(10.0f);
            }
        }
        Syn::UI::EndCard();
    }
}