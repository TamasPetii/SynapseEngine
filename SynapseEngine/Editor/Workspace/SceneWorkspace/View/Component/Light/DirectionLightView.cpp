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