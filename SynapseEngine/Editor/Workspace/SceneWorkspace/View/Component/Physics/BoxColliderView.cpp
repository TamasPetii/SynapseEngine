#include "BoxColliderView.h"
#include "Editor/Manager/EditorIcons.h"
#include "Editor/Widgets/CardWidget.h"
#include "Editor/Widgets/PropertyGrid.h"
#include <imgui.h>

namespace Syn {

    void BoxColliderView::Draw(BoxColliderViewModel& vm) {
        BoxColliderState state = vm.GetState();

        if (!state.hasComponent) return;

        constexpr const char* CardTitle = "Box Collider";

        if (Syn::UI::BeginCard(CardTitle, SYN_ICON_BOX_COLLIDER, _isCardOpen))
        {
            if (Syn::UI::BeginPropertyGrid("BoxColliderGrid"))
            {
                if (Syn::UI::PropertyDragFloat3("Half Extents", state.halfExtents, 0.1f, 0.0f, 1000.0f, "%.2f")) {
                    vm.Dispatch(SetBoxColliderHalfExtentsIntent{ state.halfExtents, !ImGui::IsItemDeactivatedAfterEdit() });
                }

                if (Syn::UI::PropertyDragFloat3("Local Offset", state.localOffset, 0.1f, -1000.0f, 1000.0f, "%.2f")) {
                    vm.Dispatch(SetBoxColliderLocalOffsetIntent{ state.localOffset, !ImGui::IsItemDeactivatedAfterEdit() });
                }

                Syn::UI::EndPropertyGrid();
            }
        }
        Syn::UI::EndCard();
    }
}