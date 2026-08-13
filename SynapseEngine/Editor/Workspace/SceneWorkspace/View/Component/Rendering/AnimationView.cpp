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

#include "AnimationView.h"
#include "Editor/Manager/EditorIcons.h"
#include "Editor/Widgets/CardWidget.h"
#include "Editor/Widgets/PropertyGrid.h"
#include <imgui.h>

namespace Syn {
    void AnimationView::Draw(AnimationViewModel& vm) {
        AnimationState state = vm.GetState();
        if (!state.hasComponent) return;

        constexpr const char* CardTitle = "Animation Component";
        if (Syn::UI::BeginCard(CardTitle, SYN_ICON_ANIMATION, _isCardOpen)) {

            if (Syn::UI::BeginPropertyGrid("AnimationGrid"))
            {
                if (Syn::UI::PropertyDragFloat("Speed", state.speed, 0.05f, 0.0f, 10.0f, "%.2f x")) {
                    vm.Dispatch(SetAnimationSpeedIntent{ state.speed, !ImGui::IsItemDeactivatedAfterEdit() });
                }

                std::string previewName = "None";
                for (const auto& anim : state.availableAnimations) {
                    if (anim.first == state.animationIndex) {
                        previewName = anim.second;
                        break;
                    }
                }

                if (Syn::UI::BeginPropertyCombo("Animation", previewName.c_str())) {
                    for (const auto& anim : state.availableAnimations) {
                        bool isSelected = (state.animationIndex == anim.first);

                        if (ImGui::Selectable(anim.second.c_str(), isSelected)) {
                            vm.Dispatch(SetAnimationIndexIntent{ anim.first });
                        }

                        if (isSelected) {
                            ImGui::SetItemDefaultFocus();
                        }
                    }
                    Syn::UI::EndPropertyCombo();
                }

                Syn::UI::EndPropertyGrid();
            }
        }
        Syn::UI::EndCard();
    }
}