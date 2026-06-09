#include "SpotLightView.h"
#include "Editor/Manager/EditorIcons.h"
#include "Editor/Widgets/CardWidget.h"
#include <imgui.h>

namespace Syn {
    void SpotLightView::Draw(SpotLightViewModel& vm) {
        SpotLightState state = vm.GetState();
        if (!state.hasComponent) return;

        constexpr const char* CardTitle = "Spot Light";
        if (Syn::UI::BeginCard(CardTitle, SYN_ICON_SPOTLIGHT, _isCardOpen)) {
            
            if (ImGui::ColorEdit3("Color", &state.color.x))
                vm.Dispatch(SetSpotLightColorIntent{ state.color, !ImGui::IsItemDeactivatedAfterEdit() });

            if (ImGui::DragFloat("Strength", &state.strength, 0.05f, 0.0f, 1000.0f, "%.2f"))
                vm.Dispatch(SetSpotLightStrengthIntent{ state.strength, !ImGui::IsItemDeactivatedAfterEdit() });

            if (ImGui::DragFloat("Range", &state.range, 0.1f, 0.0f, 1000.0f, "%.2f"))
                vm.Dispatch(SetSpotLightRangeIntent{ state.range, !ImGui::IsItemDeactivatedAfterEdit() });

            if (ImGui::DragFloat("Weaken Dist", &state.weakenDistance, 0.1f, 0.0f, 1000.0f, "%.2f"))
                vm.Dispatch(SetSpotLightWeakenIntent{ state.weakenDistance, !ImGui::IsItemDeactivatedAfterEdit() });

            if (ImGui::DragFloat("Inner Angle", &state.innerAngle, 0.1f, 0.0f, state.outerAngle, "%.1f deg"))
                vm.Dispatch(SetSpotLightInnerAngleIntent{ state.innerAngle, !ImGui::IsItemDeactivatedAfterEdit() });

            if (ImGui::DragFloat("Outer Angle", &state.outerAngle, 0.1f, state.innerAngle, 90.0f, "%.1f deg"))
                vm.Dispatch(SetSpotLightOuterAngleIntent{ state.outerAngle, !ImGui::IsItemDeactivatedAfterEdit() });

            ImGui::Spacing(); ImGui::Separator(); ImGui::Spacing();

            bool useShadow = state.useShadow;
            if (ImGui::Checkbox("Cast Shadows", &useShadow))
                vm.Dispatch(SetSpotLightUseShadowIntent{ useShadow });

            if (useShadow) {
                ImGui::Indent(10.0f);
                if (ImGui::DragFloat("Near Plane", &state.shadowNearPlane, 0.01f, 0.01f, 100.0f, "%.3f"))
                    vm.Dispatch(SetSpotLightShadowNearIntent{ state.shadowNearPlane, !ImGui::IsItemDeactivatedAfterEdit() });
                if (ImGui::DragFloat("Far Plane", &state.shadowFarPlane, 1.0f, 1.0f, 5000.0f, "%.1f"))
                    vm.Dispatch(SetSpotLightShadowFarIntent{ state.shadowFarPlane, !ImGui::IsItemDeactivatedAfterEdit() });
                ImGui::Unindent(10.0f);
            }
        }
        Syn::UI::EndCard();
    }
}