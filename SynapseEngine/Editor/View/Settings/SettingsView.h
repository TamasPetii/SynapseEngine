#pragma once
#include "../IView.h"
#include "EditorCore/ViewModels/Settings/SettingsViewModel.h"
#include <imgui.h>

namespace Syn {
    class SettingsView : public IView<SettingsViewModel> {
    public:
        void Draw(SettingsViewModel& vm) override {
            SettingsState state = vm.GetState();

            SceneSettings settings = state.sceneSettings;
            bool changed = false;

            ImGui::Begin("Scene Settings");

            ImGui::SeparatorText("General");
            changed |= ImGui::Checkbox("Debug Camera", &settings.useDebugCamera);
            changed |= ImGui::Checkbox("Transparent Picking", &settings.enableTransparentPicking);

            ImGui::SeparatorText("Rendering Features");
            changed |= ImGui::Checkbox("GPU Culling", &settings.enableGpuCulling);
            changed |= ImGui::Checkbox("Bloom", &settings.enableBloom);
            changed |= ImGui::Checkbox("Hi-Z", &settings.enableHiz);
            changed |= ImGui::Checkbox("Occlusion Culling", &settings.enableOcclusionCulling);

            ImGui::SeparatorText("Light Debug (Wireframe)");
            changed |= ImGui::Checkbox("Point Light Sphere", &settings.enablePointLightSphereWireframe);
            changed |= ImGui::Checkbox("Point Light AABB", &settings.enablePointLightAabbWireframe);
            changed |= ImGui::Checkbox("Spot Light Sphere", &settings.enableSpotLightSphereWireframe);
            changed |= ImGui::Checkbox("Spot Light AABB", &settings.enableSpotLightAabbWireframe);

            ImGui::SeparatorText("Mesh Debug (Wireframe)");
            changed |= ImGui::Checkbox("Mesh AABB", &settings.enableWireframeMeshAabb);
            changed |= ImGui::Checkbox("Mesh Sphere", &settings.enableWireframeMeshSphere);
            changed |= ImGui::Checkbox("Meshlet AABB", &settings.enableWireframeMeshletAabb);
            changed |= ImGui::Checkbox("Meshlet Sphere", &settings.enableWireframeMeshletSphere);

            ImGui::SeparatorText("Deferred Shading");
            changed |= ImGui::Checkbox("Emissive AO", &settings.enableDeferredEmissiveAo);
            changed |= ImGui::Checkbox("Point Lights", &settings.enableDeferredPointLights);
            changed |= ImGui::Checkbox("Spot Lights", &settings.enableDeferredSpotLights);
            changed |= ImGui::Checkbox("Directional Lights", &settings.enableDeferredDirectionalLights);

            if (changed) {
                vm.Dispatch(UpdateSceneSettingsIntent{ settings });
            }

            ImGui::End();
        }
    };
}