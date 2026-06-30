#include "MeshColliderView.h"
#include "Editor/Manager/EditorIcons.h" 
#include "Editor/Widgets/CardWidget.h"
#include "Editor/Widgets/PropertyGrid.h"
#include <imgui.h>

namespace Syn {

    void MeshColliderView::Draw(MeshColliderViewModel& vm) {
        MeshColliderState state = vm.GetState();

        if (!state.hasComponent) return;

        constexpr const char* CardTitle = "Mesh Collider";

        if (Syn::UI::BeginCard(CardTitle, SYN_ICON_CUBE, _isCardOpen))
        {
            if (Syn::UI::BeginPropertyGrid("MeshColliderGrid"))
            {
                Syn::UI::BeginProperty("Target LOD");
                int lodLevel = static_cast<int>(state.targetLodLevel);
                if (ImGui::SliderInt("##TargetLOD", &lodLevel, 0, 3)) {
                    vm.Dispatch(SetMeshColliderTargetLodLevelIntent{ static_cast<uint32_t>(lodLevel) });
                }

                Syn::UI::PropertySeparator();

                if (Syn::UI::PropertyDragFloat3("Local Offset", state.localOffset, 0.1f, -1000.0f, 1000.0f, "%.2f")) {
                    vm.Dispatch(SetMeshColliderLocalOffsetIntent{ state.localOffset, !ImGui::IsItemDeactivatedAfterEdit() });
                }

                Syn::UI::EndPropertyGrid();
            }
        }
        Syn::UI::EndCard();
    }
}