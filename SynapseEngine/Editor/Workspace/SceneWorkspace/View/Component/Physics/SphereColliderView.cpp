#include "SphereColliderView.h"
#include "Editor/Manager/EditorIcons.h"
#include "Editor/Widgets/CardWidget.h"
#include "Editor/Widgets/PropertyGrid.h"
#include <imgui.h>

namespace Syn {

    void SphereColliderView::Draw(SphereColliderViewModel& vm) {
        SphereColliderState state = vm.GetState();

        if (!state.hasComponent) return;

        constexpr const char* CardTitle = "Sphere Collider";

        if (Syn::UI::BeginCard(CardTitle, SYN_ICON_SPHERE_COLLIDER, _isCardOpen))
        {
            if (Syn::UI::BeginPropertyGrid("SphereColliderGrid"))
            {
                if (Syn::UI::PropertyDragFloat("Radius", state.radius, 0.1f, 0.0f, 1000.0f, "%.2f")) {
                    vm.Dispatch(SetSphereColliderRadiusIntent{ state.radius, !ImGui::IsItemDeactivatedAfterEdit() });
                }

                if (Syn::UI::PropertyDragFloat3("Local Offset", state.localOffset, 0.1f, -1000.0f, 1000.0f, "%.2f")) {
                    vm.Dispatch(SetSphereColliderLocalOffsetIntent{ state.localOffset, !ImGui::IsItemDeactivatedAfterEdit() });
                }

                Syn::UI::EndPropertyGrid();
            }
        }
        Syn::UI::EndCard();
    }
}