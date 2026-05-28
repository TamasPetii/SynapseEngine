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

            bool isCullingOpen = BeginSection("Culling & Optimization", true);
            if (isCullingOpen) {
                changed |= ImGui::Checkbox("Geometry GPU Culling", &settings.enableGeometryGpuCulling);
                changed |= ImGui::Checkbox("Static Bvh Culling", &settings.enableStaticBvhCulling);
                changed |= ImGui::Checkbox("Morton Bvh Culling", &settings.enableMortonBvhCulling);

                changed |= ImGui::Checkbox("Hi-Z", &settings.enableHiz);

                ImGui::Spacing();
                ImGui::Separator();
                ImGui::Spacing();

                changed |= ImGui::Checkbox("Global Frustum Culling", &settings.enableFrustumCulling);
                changed |= ImGui::Checkbox("Global Occlusion Culling", &settings.enableOcclusionCulling);

                ImGui::Spacing();
                ImGui::SeparatorText("Chunk Level");

                ImGui::BeginDisabled(!settings.enableFrustumCulling);
                changed |= ImGui::Checkbox("Frustum Culling##Chunk", &settings.enableChunkFrustumCulling);
                ImGui::EndDisabled();

                ImGui::BeginDisabled(!settings.enableOcclusionCulling);
                changed |= ImGui::Checkbox("Occlusion Culling##Chunk", &settings.enableChunkOcclusionCulling);
                ImGui::EndDisabled();

                ImGui::Spacing();
                ImGui::SeparatorText("Model Level");

                ImGui::BeginDisabled(!settings.enableFrustumCulling);
                changed |= ImGui::Checkbox("Frustum Culling##Model", &settings.enableModelFrustumCulling);
                ImGui::EndDisabled();

                ImGui::BeginDisabled(!settings.enableOcclusionCulling);
                changed |= ImGui::Checkbox("Occlusion Culling##Model", &settings.enableModelOcclusionCulling);
                ImGui::EndDisabled();

                ImGui::Spacing();
                ImGui::SeparatorText("Mesh Level");

                ImGui::BeginDisabled(!settings.enableFrustumCulling);
                changed |= ImGui::Checkbox("Frustum Culling##Mesh", &settings.enableMeshFrustumCulling);
                ImGui::EndDisabled();

                ImGui::BeginDisabled(!settings.enableOcclusionCulling);
                changed |= ImGui::Checkbox("Occlusion Culling##Mesh", &settings.enableMeshOcclusionCulling);
                ImGui::EndDisabled();

                ImGui::Spacing();
                ImGui::SeparatorText("Meshlet Level");

                ImGui::BeginDisabled(!settings.enableFrustumCulling);
                changed |= ImGui::Checkbox("Frustum Culling##Meshlet", &settings.enableMeshletFrustumCulling);
                ImGui::EndDisabled();

                ImGui::BeginDisabled(!settings.enableOcclusionCulling);
                changed |= ImGui::Checkbox("Occlusion Culling##Meshlet", &settings.enableMeshletOcclusionCulling);
                ImGui::EndDisabled();

                changed |= ImGui::Checkbox("Cone Culling##Meshlet", &settings.enableMeshletConeCulling);

                ImGui::Spacing();
                ImGui::SeparatorText("Point Light Level");

                changed |= ImGui::Checkbox("Point Light GPU Culling", &settings.enablePointLightGpuCulling);

                ImGui::BeginDisabled(!settings.enableFrustumCulling);
                changed |= ImGui::Checkbox("Frustum Culling##PointLight", &settings.enablePointLightFrustumCulling);
                ImGui::EndDisabled();

                ImGui::BeginDisabled(!settings.enableOcclusionCulling);
                changed |= ImGui::Checkbox("Occlusion Culling##PointLight", &settings.enablePointLightOcclusionCulling);
                ImGui::EndDisabled();

                ImGui::Spacing();
                ImGui::SeparatorText("Spot Light Level");

                changed |= ImGui::Checkbox("Spot Light GPU Culling", &settings.enableSpotLightGpuCulling);

                ImGui::BeginDisabled(!settings.enableFrustumCulling);
                changed |= ImGui::Checkbox("Frustum Culling##SpotLight", &settings.enableSpotLightFrustumCulling);
                ImGui::EndDisabled();

                ImGui::BeginDisabled(!settings.enableOcclusionCulling);
                changed |= ImGui::Checkbox("Occlusion Culling##SpotLight", &settings.enableSpotLightOcclusionCulling);
                ImGui::EndDisabled();
            }
            EndSection(isCullingOpen);

            bool isBloomOpen = BeginSection("Post-Processing");
            if (isBloomOpen) {
				ImGui::SeparatorText("Bloom");
                changed |= ImGui::Checkbox("Enable Bloom", &settings.enableBloom);
                if (settings.enableBloom) {
                    changed |= ImGui::DragFloat("Threshold", &settings.bloomThreshold, 0.01f, 0.0f, 10.0f);
                    changed |= ImGui::DragFloat("Knee", &settings.bloomKnee, 0.01f, 0.0f, 1.0f);
                    changed |= ImGui::DragFloat("Filter Radius", &settings.bloomFilterRadius, 0.001f, 0.0f, 0.1f, "%.4f");
                    changed |= ImGui::DragFloat("Exposure", &settings.bloomExposure, 0.01f, 0.1f, 10.0f);
                    changed |= ImGui::DragFloat("Strength", &settings.bloomStrength, 0.01f, 0.0f, 5.0f);
                }

				ImGui::SeparatorText("SSAO");
				changed |= ImGui::Checkbox("Enable SSAO", &settings.enableSsao);
				if (settings.enableSsao) {
					changed |= ImGui::Checkbox("Enable Light SSAO", &settings.enableSsaoLight);
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

            bool isForwardPlusOpen = BeginSection("Forward Plus Shading Features", false);
            if (isForwardPlusOpen) {
                changed |= ImGui::Checkbox("Emissive AO##ForwardPlus", &settings.enableForwardPlusEmissiveAo);
                changed |= ImGui::Checkbox("Point Lights##ForwardPlus", &settings.enableForwardPlusPointLights);
                changed |= ImGui::Checkbox("Spot Lights##ForwardPlus", &settings.enableForwardPlusSpotLights);
                changed |= ImGui::Checkbox("Directional Lights##ForwardPlus", &settings.enableForwardPlusDirectionalLights);
            }
            EndSection(isForwardPlusOpen);

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
                changed |= ImGui::Checkbox("Static Chunk AABB", &settings.enableStaticChunkAabbWireframe);
            }
            EndSection(isDebugOpen);

            bool isSsaoOpen = BeginSection("DP-HVO (SSAO)", false);
            if (isSsaoOpen) {
                ImGui::SeparatorText("Parameters");
                changed |= ImGui::DragFloat("Radius##Ssao", &settings.aoRadius, 0.01f, 0.0f, 100.f);
                changed |= ImGui::DragFloat("Intensity##Ssao", &settings.aoIntensity, 0.1f, 0.0f, 100.f);
                changed |= ImGui::DragFloat("Max Distance##Ssao", &settings.maxOcclusionDistance, 0.1f, 0.0f, 100.f);
                changed |= ImGui::DragFloat("Depth Sharpness##Ssao", &settings.depthSharpness, 0.05f, 0.0f, 100.f);
                changed |= ImGui::DragFloat("Bias##Ssao", &settings.bias, 0.001f, 0.0f, 100.f);
                changed |= ImGui::DragInt("Sample Count##Ssao", &settings.sampleCount, 1.0f, 1, 100);
                

            }
            EndSection(isSsaoOpen);

            if (changed) {
                vm.Dispatch(UpdateSceneSettingsIntent{ settings });
            }

            ImGui::End();
        }
    };
}