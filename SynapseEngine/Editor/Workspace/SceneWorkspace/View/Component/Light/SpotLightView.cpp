#include "SpotLightView.h"
#include "Editor/Manager/EditorIcons.h"
#include "Editor/Widgets/CardWidget.h"
#include "Editor/Widgets/PropertyGrid.h"
#include <imgui.h>

namespace Syn {
    void SpotLightView::Draw(SpotLightViewModel& vm) {
        SpotLightState state = vm.GetState();
        if (!state.hasComponent) return;

        constexpr const char* CardTitle = "Spot Light";
        if (Syn::UI::BeginCard(CardTitle, SYN_ICON_SPOTLIGHT, _isCardOpen)) {

            if (Syn::UI::BeginPropertyGrid("SpotLightGrid"))
            {
                if (Syn::UI::PropertyColor3("Color", state.color))
                    vm.Dispatch(SetSpotLightColorIntent{ state.color, !ImGui::IsItemDeactivatedAfterEdit() });

                if (Syn::UI::PropertyDragFloat("Strength", state.strength, 0.05f, 0.0f, 1000.0f, "%.2f"))
                    vm.Dispatch(SetSpotLightStrengthIntent{ state.strength, !ImGui::IsItemDeactivatedAfterEdit() });

                if (Syn::UI::PropertyDragFloat("Range", state.range, 0.1f, 0.0f, 1000.0f, "%.2f"))
                    vm.Dispatch(SetSpotLightRangeIntent{ state.range, !ImGui::IsItemDeactivatedAfterEdit() });

                if (Syn::UI::PropertyDragFloat("Weaken Dist", state.weakenDistance, 0.1f, 0.0f, 1000.0f, "%.2f"))
                    vm.Dispatch(SetSpotLightWeakenIntent{ state.weakenDistance, !ImGui::IsItemDeactivatedAfterEdit() });

                if (Syn::UI::PropertyDragFloat("Inner Angle", state.innerAngle, 0.1f, 0.0f, state.outerAngle, "%.1f deg"))
                    vm.Dispatch(SetSpotLightInnerAngleIntent{ state.innerAngle, !ImGui::IsItemDeactivatedAfterEdit() });

                if (Syn::UI::PropertyDragFloat("Outer Angle", state.outerAngle, 0.1f, state.innerAngle, 90.0f, "%.1f deg"))
                    vm.Dispatch(SetSpotLightOuterAngleIntent{ state.outerAngle, !ImGui::IsItemDeactivatedAfterEdit() });

                Syn::UI::PropertySeparator();

                bool useShadow = state.useShadow;
                if (Syn::UI::PropertyCheckbox("Cast Shadows", useShadow))
                    vm.Dispatch(SetSpotLightUseShadowIntent{ useShadow });

                if (useShadow) {
                    if (Syn::UI::PropertyDragFloat("Near Plane", state.shadowNearPlane, 0.01f, 0.01f, 100.0f, "%.3f", 1))
                        vm.Dispatch(SetSpotLightShadowNearIntent{ state.shadowNearPlane, !ImGui::IsItemDeactivatedAfterEdit() });

                    if (Syn::UI::PropertyDragFloat("Far Plane", state.shadowFarPlane, 1.0f, 1.0f, 5000.0f, "%.1f", 1))
                        vm.Dispatch(SetSpotLightShadowFarIntent{ state.shadowFarPlane, !ImGui::IsItemDeactivatedAfterEdit() });
                }

                Syn::UI::EndPropertyGrid();
            }
        }
        Syn::UI::EndCard();
    }
}