#include "PointLightView.h"
#include "Editor/Manager/EditorIcons.h"
#include "Editor/Widgets/CardWidget.h"
#include "Editor/Widgets/PropertyGrid.h"
#include <imgui.h>

namespace Syn {

    void PointLightView::Draw(PointLightViewModel& vm) {
        PointLightState state = vm.GetState();

        if (!state.hasComponent) return;

        constexpr const char* CardTitle = "Point Light";

        if (Syn::UI::BeginCard(CardTitle, SYN_ICON_LIGHTBULB, _isCardOpen))
        {
            if (Syn::UI::BeginPropertyGrid("PointLightGrid"))
            {
                if (Syn::UI::PropertyColor3("Color", state.color)) {
                    vm.Dispatch(SetPointLightColorIntent{ state.color, !ImGui::IsItemDeactivatedAfterEdit() });
                }

                if (Syn::UI::PropertyDragFloat("Strength", state.strength, 0.05f, 0.0f, 1000.0f, "%.2f")) {
                    vm.Dispatch(SetPointLightStrengthIntent{ state.strength, !ImGui::IsItemDeactivatedAfterEdit() });
                }

                if (Syn::UI::PropertyDragFloat("Radius", state.radius, 0.1f, 0.0f, 1000.0f, "%.2f")) {
                    vm.Dispatch(SetPointLightRadiusIntent{ state.radius, !ImGui::IsItemDeactivatedAfterEdit() });
                }

                if (Syn::UI::PropertyDragFloat("Weaken Dist", state.weakenDistance, 0.1f, 0.0f, 1000.0f, "%.2f")) {
                    vm.Dispatch(SetPointLightWeakenIntent{ state.weakenDistance, !ImGui::IsItemDeactivatedAfterEdit() });
                }

                Syn::UI::PropertySeparator();

                bool useShadow = state.useShadow;
                if (Syn::UI::PropertyCheckbox("Cast Shadows", useShadow)) {
                    vm.Dispatch(SetPointLightUseShadowIntent{ useShadow });
                }

                if (useShadow) {
                    if (Syn::UI::PropertyDragFloat("Near Plane", state.shadowNearPlane, 0.01f, 0.01f, 100.0f, "%.3f", 1)) {
                        vm.Dispatch(SetPointLightShadowNearIntent{ state.shadowNearPlane, !ImGui::IsItemDeactivatedAfterEdit() });
                    }

                    if (Syn::UI::PropertyDragFloat("Far Plane", state.shadowFarPlane, 1.0f, 1.0f, 5000.0f, "%.1f", 1)) {
                        vm.Dispatch(SetPointLightShadowFarIntent{ state.shadowFarPlane, !ImGui::IsItemDeactivatedAfterEdit() });
                    }
                }

                Syn::UI::EndPropertyGrid();
            }
        }
        Syn::UI::EndCard();
    }
}