#pragma once
#include "../IView.h"
#include "EditorCore/ViewModels/Transform/TransformViewModel.h"
#include <imgui.h>

namespace Syn {
    class TransformView : public IView<TransformViewModel> {
    public:
        void Draw(TransformViewModel& vm) override {
            TransformState state = vm.GetState();

            ImGui::Begin("Transform");

            if (!state.hasSelection) {
                ImGui::TextDisabled("No entity selected.");
                ImGui::End();
                return;
            }

            ImGui::Text("Entity ID: %d", state.activeEntityId);
            ImGui::Separator();

            bool posChanged = ImGui::DragFloat3("Position", &state.position.x, 0.1f);
            bool posDeactivated = ImGui::IsItemDeactivatedAfterEdit();
            if (posChanged || posDeactivated) {
                vm.Dispatch(SetPositionIntent{ state.position, !posDeactivated });
            }


            bool rotChanged = ImGui::DragFloat3("Rotation", &state.rotation.x, 0.5f);
            bool rotDeactivated = ImGui::IsItemDeactivatedAfterEdit();
            if (rotChanged || rotDeactivated) {
                vm.Dispatch(SetRotationIntent{ state.rotation, !rotDeactivated });
            }


            bool scaleChanged = ImGui::DragFloat3("Scale", &state.scale.x, 0.05f);
            bool scaleDeactivated = ImGui::IsItemDeactivatedAfterEdit();
            if (scaleChanged || scaleDeactivated) {
                vm.Dispatch(SetScaleIntent{ state.scale, !scaleDeactivated });
            }

            ImGui::End();
        }
    };
}