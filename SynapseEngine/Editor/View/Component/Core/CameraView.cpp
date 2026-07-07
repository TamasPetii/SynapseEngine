#include "CameraView.h"
#include "Editor/Manager/EditorIcons.h"
#include "Editor/Widgets/CardWidget.h"
#include "Editor/Widgets/PropertyGrid.h"
#include <imgui.h>

namespace Syn {

    void CameraView::Draw(CameraViewModel& vm) {
        CameraState state = vm.GetState();

        if (!state.hasComponent) return;

        constexpr const char* CardTitle = "Camera";

        if (Syn::UI::BeginCard(CardTitle, SYN_ICON_CAMERA, _isCardOpen))
        {
            if (Syn::UI::BeginPropertyGrid("CameraGrid"))
            {
                if (Syn::UI::PropertyCheckbox("Orthographic", state.isOrthographic)) {
                    vm.Dispatch(SetCameraIsOrthographicIntent{ state.isOrthographic });
                }

                if (Syn::UI::PropertyCheckbox("Orbit Camera", state.useOrbit)) {
                    vm.Dispatch(SetCameraUseOrbitIntent{ state.useOrbit });
                }

                Syn::UI::PropertySeparator();

                if (Syn::UI::PropertyDragFloat("Yaw", state.yaw, 0.1f, -360.0f, 360.0f, "%.2f")) {
                    vm.Dispatch(SetCameraYawIntent{ state.yaw, !ImGui::IsItemDeactivatedAfterEdit() });
                }

                if (Syn::UI::PropertyDragFloat("Pitch", state.pitch, 0.1f, -89.0f, 89.0f, "%.2f")) {
                    vm.Dispatch(SetCameraPitchIntent{ state.pitch, !ImGui::IsItemDeactivatedAfterEdit() });
                }

                Syn::UI::PropertySeparator();

                if (state.isOrthographic)
                {
                    if (Syn::UI::PropertyDragFloat("Ortho Size", state.orthoSize, 0.1f, 1.0f, 1000.0f, "%.2f")) {
                        vm.Dispatch(SetCameraOrthoSizeIntent{ state.orthoSize, !ImGui::IsItemDeactivatedAfterEdit() });
                    }
                }
                else
                {
                    if (Syn::UI::PropertyDragFloat("FOV", state.fov, 0.1f, 1.0f, 179.0f, "%.2f")) {
                        vm.Dispatch(SetCameraFovIntent{ state.fov, !ImGui::IsItemDeactivatedAfterEdit() });
                    }
                }

                if (Syn::UI::PropertyDragFloat("Near Plane", state.nearPlane, 0.01f, 0.001f, 100.0f, "%.3f")) {
                    vm.Dispatch(SetCameraNearPlaneIntent{ state.nearPlane, !ImGui::IsItemDeactivatedAfterEdit() });
                }

                if (Syn::UI::PropertyDragFloat("Far Plane", state.farPlane, 1.0f, 1.0f, 10000.0f, "%.1f")) {
                    vm.Dispatch(SetCameraFarPlaneIntent{ state.farPlane, !ImGui::IsItemDeactivatedAfterEdit() });
                }

                Syn::UI::PropertySeparator();

                if (Syn::UI::PropertyDragFloat("Speed", state.speed, 0.1f, 0.0f, 100.0f, "%.2f")) {
                    vm.Dispatch(SetCameraSpeedIntent{ state.speed, !ImGui::IsItemDeactivatedAfterEdit() });
                }

                if (Syn::UI::PropertyDragFloat("Sensitivity", state.sensitivity, 0.01f, 0.0f, 10.0f, "%.3f")) {
                    vm.Dispatch(SetCameraSensitivityIntent{ state.sensitivity, !ImGui::IsItemDeactivatedAfterEdit() });
                }

                if (Syn::UI::PropertyDragFloat("Distance", state.distance, 0.1f, 0.0f, 500.0f, "%.2f")) {
                    vm.Dispatch(SetCameraDistanceIntent{ state.distance, !ImGui::IsItemDeactivatedAfterEdit() });
                }

                Syn::UI::EndPropertyGrid();
            }
        }
        Syn::UI::EndCard();
    }
}