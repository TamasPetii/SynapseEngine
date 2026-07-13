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