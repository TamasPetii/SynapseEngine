#include "DirectionLightView.h"
#include "Editor/Manager/EditorIcons.h"
#include "Editor/Widgets/CardWidget.h"
#include "Editor/Widgets/PropertyGrid.h"
#include <imgui.h>

namespace Syn {

    void DirectionLightView::Draw(DirectionLightViewModel& vm) {
        DirectionLightState state = vm.GetState();

        if (!state.hasComponent) return;

        constexpr const char* CardTitle = "Directional Light";
        if (Syn::UI::BeginCard(CardTitle, SYN_ICON_SUN, _isCardOpen))
        {
            if (Syn::UI::BeginPropertyGrid("LightPropsGrid"))
            {
                if (Syn::UI::PropertyColor3("Color", state.color)) {
                    vm.Dispatch(SetLightColorIntent{ state.color, !ImGui::IsItemDeactivatedAfterEdit() });
                }

                if (Syn::UI::PropertyDragFloat("Strength", state.strength, 0.05f, 0.0f, 1000.0f, "%.2f")) {
                    vm.Dispatch(SetLightStrengthIntent{ state.strength, !ImGui::IsItemDeactivatedAfterEdit() });
                }

                Syn::UI::PropertySeparator();

                bool useShadow = state.useShadow;
                if (Syn::UI::PropertyCheckbox("Cast Shadows", useShadow)) {
                    vm.Dispatch(SetLightUseShadowIntent{ useShadow });
                }

                if (useShadow) {
                    if (Syn::UI::PropertyDragFloat("Shadow Distance", state.shadowFarPlane, 1.0f, 10.0f, 5000.0f, "%.1f", 1)) {
                        vm.Dispatch(SetShadowFarPlaneIntent{ state.shadowFarPlane, !ImGui::IsItemDeactivatedAfterEdit() });
                    }

                    if (Syn::UI::PropertyDragFloat4("Cascade Splits", state.cascadeSplits, 0.01f, 0.0f, 1.0f, "%.3f", 1)) {
                        vm.Dispatch(SetCascadeSplitsIntent{ state.cascadeSplits, !ImGui::IsItemDeactivatedAfterEdit() });
                    }
                }

                Syn::UI::EndPropertyGrid();
            }
        }
        Syn::UI::EndCard();
    }
}