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