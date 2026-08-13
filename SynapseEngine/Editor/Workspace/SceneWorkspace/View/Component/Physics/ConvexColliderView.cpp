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

#include "ConvexColliderView.h"
#include "Editor/Manager/EditorIcons.h" 
#include "Editor/Widgets/CardWidget.h"
#include "Editor/Widgets/PropertyGrid.h"
#include <imgui.h>

namespace Syn {

    void ConvexColliderView::Draw(ConvexColliderViewModel& vm) {
        ConvexColliderState state = vm.GetState();

        if (!state.hasComponent) return;

        constexpr const char* CardTitle = "Convex Collider";

        if (Syn::UI::BeginCard(CardTitle, SYN_ICON_CONVEX_COLLIDER, _isCardOpen))
        {
            if (Syn::UI::BeginPropertyGrid("ConvexColliderGrid"))
            {
                Syn::UI::BeginProperty("Target LOD");
                int lodLevel = static_cast<int>(state.targetLodLevel);
                if (ImGui::SliderInt("##TargetLOD", &lodLevel, 0, 3)) {
                    vm.Dispatch(SetConvexColliderTargetLodLevelIntent{ static_cast<uint32_t>(lodLevel) });
                }

                Syn::UI::PropertySeparator();

                if (Syn::UI::PropertyDragFloat3("Local Offset", state.localOffset, 0.1f, -1000.0f, 1000.0f, "%.2f")) {
                    vm.Dispatch(SetConvexColliderLocalOffsetIntent{ state.localOffset, !ImGui::IsItemDeactivatedAfterEdit() });
                }

                Syn::UI::EndPropertyGrid();
            }
        }
        Syn::UI::EndCard();
    }
}