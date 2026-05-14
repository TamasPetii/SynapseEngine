#pragma once
#include "../IView.h"
#include "EditorCore/ViewModels/Settings/SettingsViewModel.h"
#include <imgui.h>
#include <string>
#include "Engine/Render/ComputeGroupSize.h"

namespace Syn {
    class SettingsView : public IView<SettingsViewModel> {
    public:
        void Draw(SettingsViewModel& vm) override {
            SettingsState state = vm.GetState();
            SceneSettings settings = state.sceneSettings;
            bool changed = false;

            ImGui::Begin("Scene Settings");

            auto BeginSection = [](const char* label, bool defaultOpen = true) -> bool {
                ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_Framed | ImGuiTreeNodeFlags_NoTreePushOnOpen;
                if (defaultOpen) flags |= ImGuiTreeNodeFlags_DefaultOpen;

                bool isOpen = ImGui::TreeNodeEx(label, flags);
                if (isOpen) {
                    ImGui::Spacing();
                    ImGui::Indent(10.0f);
                }
                return isOpen;
                };

            auto EndSection = [](bool isOpen) {
                if (isOpen) {
                    ImGui::Unindent(10.0f);
                    ImGui::Spacing();
                }
                ImGui::Spacing();
                };

            bool isGeneralOpen = BeginSection("General & Rendering Pipeline");
            if (isGeneralOpen) {
                changed |= ImGui::Checkbox("Debug Camera", &settings.useDebugCamera);

                const char* pipelineNames[] = { "Deferred", "Forward+" };
                int currentPipeline = (int)settings.pipelineType;
                if (ImGui::Combo("Pipeline Type", &currentPipeline, pipelineNames, IM_ARRAYSIZE(pipelineNames))) {
                    settings.pipelineType = (PipelineType)currentPipeline;
                    changed = true;
                }

                if (currentPipeline == (int)PipelineType::ForwardPlus) {

                    const char* sliderNames[] = { "8", "16", "32", "64", "128", "256", "512" };

                    const uint32_t sizes[] = {
                        ComputeGroupSize::Image8D,
                        ComputeGroupSize::Image16D,
                        ComputeGroupSize::Image32D,
                        ComputeGroupSize::Image64D,
                        ComputeGroupSize::Image128D,
                        ComputeGroupSize::Image256D,
                        ComputeGroupSize::Image512D
                    };

                    int currentTileSizeIndex = 0;
                    for (int i = 0; i < IM_ARRAYSIZE(sizes); ++i) {
                        if (settings.tileSize == sizes[i]) {
                            currentTileSizeIndex = i;
                            break;
                        }
                    }

                    if (ImGui::Combo("Tile Size", &currentTileSizeIndex, sliderNames, IM_ARRAYSIZE(sliderNames))) {
                        settings.tileSize = sizes[currentTileSizeIndex];
                        changed = true;
                    }
                }

                changed |= ImGui::SliderFloat("Ambient Strength", &settings.ambientStrength, 0.0f, 1.0f);
                changed |= ImGui::SliderFloat("Emissive Strength", &settings.emissiveStrength, 0.0f, 10.0f);
            }
            EndSection(isGeneralOpen);

            bool isCullingOpen = BeginSection("Culling & Optimization", false);
            if (isCullingOpen) {
                changed |= ImGui::Checkbox("GPU Culling", &settings.enableGpuCulling);
                changed |= ImGui::Checkbox("Hi-Z", &settings.enableHiz);
                changed |= ImGui::Checkbox("Occlusion Culling", &settings.enableOcclusionCulling);
                changed |= ImGui::Checkbox("Frustum Culling", &settings.enableFrustumCulling);
                changed |= ImGui::Checkbox("Cone Culling", &settings.enableConeCulling);
            }
            EndSection(isCullingOpen);

            bool isBloomOpen = BeginSection("Post-Processing (Bloom)");
            if (isBloomOpen) {
                changed |= ImGui::Checkbox("Enable Bloom", &settings.enableBloom);
                if (settings.enableBloom) {
                    changed |= ImGui::DragFloat("Threshold", &settings.bloomThreshold, 0.01f, 0.0f, 10.0f);
                    changed |= ImGui::DragFloat("Knee", &settings.bloomKnee, 0.01f, 0.0f, 1.0f);
                    changed |= ImGui::DragFloat("Filter Radius", &settings.bloomFilterRadius, 0.001f, 0.0f, 0.1f, "%.4f");
                    changed |= ImGui::DragFloat("Exposure", &settings.bloomExposure, 0.01f, 0.1f, 10.0f);
                    changed |= ImGui::DragFloat("Strength", &settings.bloomStrength, 0.01f, 0.0f, 5.0f);
                }
            }
            EndSection(isBloomOpen);

            bool isDeferredOpen = BeginSection("Deferred Shading Features", false);
            if (isDeferredOpen) {
                changed |= ImGui::Checkbox("Emissive AO##Deferred", &settings.enableDeferredEmissiveAo);
                changed |= ImGui::Checkbox("Point Lights##Deferred", &settings.enableDeferredPointLights);
                changed |= ImGui::Checkbox("Spot Lights##Deferred", &settings.enableDeferredSpotLights);
                changed |= ImGui::Checkbox("Directional Lights##Deferred", &settings.enableDeferredDirectionalLights);
            }
            EndSection(isDeferredOpen);

            bool isDebugOpen = BeginSection("Debug Visualization", false);
            if (isDebugOpen) {
                ImGui::SeparatorText("Billboards");
                changed |= ImGui::Checkbox("Cameras##Billboards", &settings.enableBillboardCameras);
                changed |= ImGui::Checkbox("Point Lights##Billboards", &settings.enableBillboardPointLights);
                changed |= ImGui::Checkbox("Spot Lights##Billboards", &settings.enableBillboardSpotLights);
                changed |= ImGui::Checkbox("Directional Lights##Billboards", &settings.enableBillboardDirectionalLights);

                ImGui::SeparatorText("Light Wireframes");
                changed |= ImGui::Checkbox("Point Light Sphere", &settings.enablePointLightSphereWireframe);
                changed |= ImGui::Checkbox("Point Light AABB", &settings.enablePointLightAabbWireframe);
                changed |= ImGui::Checkbox("Spot Light Sphere", &settings.enableSpotLightSphereWireframe);
                changed |= ImGui::Checkbox("Spot Light AABB", &settings.enableSpotLightAabbWireframe);
                changed |= ImGui::Checkbox("Spot Light Cone", &settings.enableSpotLightConeWireframe);
                changed |= ImGui::Checkbox("Spot Light Pyramid", &settings.enableSpotLightPyramidWireframe);

                ImGui::SeparatorText("Mesh Wireframes");
                changed |= ImGui::Checkbox("Mesh AABB", &settings.enableWireframeMeshAabb);
                changed |= ImGui::Checkbox("Mesh Sphere", &settings.enableWireframeMeshSphere);
            }
            EndSection(isDebugOpen);

            if (changed) {
                vm.Dispatch(UpdateSceneSettingsIntent{ settings });
            }

            ImGui::End();
        }
    };
}