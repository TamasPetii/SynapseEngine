// Copyright (C) 2026 Tamás Péter
// This file is part of SynapseEngine.
//
// SynapseEngine is free software: you can redistribute it and/or modify
// it under the terms of the GNU Affero General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// SynapseEngine is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License
// along with SynapseEngine. If not, see <https://www.gnu.org/licenses/>.

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