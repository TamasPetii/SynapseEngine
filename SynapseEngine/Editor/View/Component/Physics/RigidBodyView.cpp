#include "RigidBodyView.h"
#include "Editor/Manager/EditorIcons.h" 
#include "Editor/Widgets/CardWidget.h"
#include "Editor/Widgets/PropertyGrid.h"
#include <imgui.h>

namespace Syn {

    void RigidBodyView::Draw(RigidBodyViewModel& vm) {
        RigidBodyState state = vm.GetState();

        if (!state.hasComponent) return;

        constexpr const char* CardTitle = "Rigid Body";

        if (Syn::UI::BeginCard(CardTitle, SYN_ICON_CUBE, _isCardOpen))
        {
            if (Syn::UI::BeginPropertyGrid("RigidBodyGrid"))
            {
                const char* currentMotionString = "Dynamic";
                if (state.motionType == PhysicsMotionType::Static) currentMotionString = "Static";
                else if (state.motionType == PhysicsMotionType::Kinematic) currentMotionString = "Kinematic";

                if (Syn::UI::BeginPropertyCombo("Motion Type", currentMotionString)) {
                    if (ImGui::Selectable("Static", state.motionType == PhysicsMotionType::Static)) {
                        vm.Dispatch(SetRigidBodyMotionTypeIntent{ PhysicsMotionType::Static });
                    }
                    if (ImGui::Selectable("Kinematic", state.motionType == PhysicsMotionType::Kinematic)) {
                        vm.Dispatch(SetRigidBodyMotionTypeIntent{ PhysicsMotionType::Kinematic });
                    }
                    if (ImGui::Selectable("Dynamic", state.motionType == PhysicsMotionType::Dynamic)) {
                        vm.Dispatch(SetRigidBodyMotionTypeIntent{ PhysicsMotionType::Dynamic });
                    }
                    Syn::UI::EndPropertyCombo();
                }

                Syn::UI::PropertySeparator();
      
                if (Syn::UI::PropertyDragFloat("Mass", state.mass, 0.1f, 0.0f, 10000.0f, "%.2f")) {
                    vm.Dispatch(SetRigidBodyMassIntent{ state.mass, !ImGui::IsItemDeactivatedAfterEdit() });
                }

                if (Syn::UI::PropertyDragFloat("Friction", state.friction, 0.01f, 0.0f, 1.0f, "%.2f")) {
                    vm.Dispatch(SetRigidBodyFrictionIntent{ state.friction, !ImGui::IsItemDeactivatedAfterEdit() });
                }

                if (Syn::UI::PropertyDragFloat("Restitution", state.restitution, 0.01f, 0.0f, 1.0f, "%.2f")) {
                    vm.Dispatch(SetRigidBodyRestitutionIntent{ state.restitution, !ImGui::IsItemDeactivatedAfterEdit() });
                }

                Syn::UI::PropertySeparator();

                Syn::UI::BeginProperty("Layer");
                int layerInt = static_cast<int>(state.layer);
                if (ImGui::DragInt("##Layer", &layerInt, 1.0f, 0, 31)) {
                    vm.Dispatch(SetRigidBodyLayerIntent{ static_cast<uint32_t>(layerInt) });
                }

                Syn::UI::EndPropertyGrid();
            }
        }
        Syn::UI::EndCard();
    }
}