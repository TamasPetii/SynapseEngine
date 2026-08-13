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

#include "CapsuleColliderView.h"
#include "Editor/Manager/EditorIcons.h" 
#include "Editor/Widgets/CardWidget.h"
#include "Editor/Widgets/PropertyGrid.h"
#include <imgui.h>

namespace Syn {

    void CapsuleColliderView::Draw(CapsuleColliderViewModel& vm) {
        CapsuleColliderState state = vm.GetState();

        if (!state.hasComponent) return;

        constexpr const char* CardTitle = "Capsule Collider";

        if (Syn::UI::BeginCard(CardTitle, SYN_ICON_CAPSULE_COLLIDER, _isCardOpen))
        {
            if (Syn::UI::BeginPropertyGrid("CapsuleColliderGrid"))
            {
                if (Syn::UI::PropertyDragFloat("Radius", state.radius, 0.1f, 0.0f, 1000.0f, "%.2f")) {
                    vm.Dispatch(SetCapsuleColliderRadiusIntent{ state.radius, !ImGui::IsItemDeactivatedAfterEdit() });
                }

                if (Syn::UI::PropertyDragFloat("Half Height", state.halfHeight, 0.1f, 0.0f, 1000.0f, "%.2f")) {
                    vm.Dispatch(SetCapsuleColliderHalfHeightIntent{ state.halfHeight, !ImGui::IsItemDeactivatedAfterEdit() });
                }

                if (Syn::UI::PropertyDragFloat3("Local Offset", state.localOffset, 0.1f, -1000.0f, 1000.0f, "%.2f")) {
                    vm.Dispatch(SetCapsuleColliderLocalOffsetIntent{ state.localOffset, !ImGui::IsItemDeactivatedAfterEdit() });
                }

                Syn::UI::EndPropertyGrid();
            }
        }
        Syn::UI::EndCard();
    }
}