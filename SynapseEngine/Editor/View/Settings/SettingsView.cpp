#include "SettingsView.h"
#include "Editor/Manager/EditorIcons.h"
#include "Engine/Render/ComputeGroupSize.h"
#include "Editor/Widgets/CardWidget.h"
#include <imgui.h>
#include <string>

namespace Syn {

    void SettingsView::Draw(SettingsViewModel& vm) {
        SettingsState state = vm.GetState();
        SceneSettings settings = state.sceneSettings;
        bool changed = false;

        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(12, 12));

        if (ImGui::Begin(SYN_ICON_SLIDERS_H " Graphics & Environment")) {

            auto getCardState = [this](const std::string& name) -> bool& {
                if (_cardStates.find(name) == _cardStates.end()) _cardStates[name] = true;
                return _cardStates[name];
                };

            constexpr const char* CardGlobalPipelineTitle = "Global Pipeline";
            if (Syn::UI::BeginCard(CardGlobalPipelineTitle, SYN_ICON_GLOBE, getCardState(CardGlobalPipelineTitle))) {

                const char* pipelineNames[] = { "Deferred", "Forward+" };
                int currentPipeline = (int)settings.pipelineType;

                ImGui::SetNextItemWidth(200.0f);
                if (ImGui::Combo("Pipeline Architecture", &currentPipeline, pipelineNames, IM_ARRAYSIZE(pipelineNames))) {
                    settings.pipelineType = (PipelineType)currentPipeline;
                    changed = true;
                }

                if (settings.pipelineType == PipelineType::ForwardPlus) {
                    const char* sliderNames[] = { "8", "16", "32", "64", "128", "256", "512" };
                    const uint32_t sizes[] = {
                        ComputeGroupSize::Image8D, ComputeGroupSize::Image16D, ComputeGroupSize::Image32D,
                        ComputeGroupSize::Image64D, ComputeGroupSize::Image128D, ComputeGroupSize::Image256D,
                        ComputeGroupSize::Image512D
                    };

                    int currentTileSizeIndex = 0;
                    for (int i = 0; i < IM_ARRAYSIZE(sizes); ++i) {
                        if (settings.tileSize == sizes[i]) {
                            currentTileSizeIndex = i; break;
                        }
                    }

                    ImGui::SetNextItemWidth(200.0f);
                    if (ImGui::Combo("Compute Tile Size", &currentTileSizeIndex, sliderNames, IM_ARRAYSIZE(sliderNames))) {
                        settings.tileSize = sizes[currentTileSizeIndex];
                        changed = true;
                    }
                }

                ImGui::Spacing();
                changed |= ImGui::SliderFloat("Ambient Strength", &settings.ambientStrength, 0.0f, 1.0f);
                changed |= ImGui::SliderFloat("Emissive Strength", &settings.emissiveStrength, 0.0f, 10.0f);
            }
            Syn::UI::EndCard();  

            constexpr const char* CardCullingTitle = "Culling & Optimization";
            if (Syn::UI::BeginCard(CardCullingTitle, SYN_ICON_CROP, getCardState(CardCullingTitle))) {

                ImGui::TextDisabled("Spatial Acceleration");
                changed |= ImGui::Checkbox("Static BVH", &settings.enableStaticBvhCulling);
                ImGui::SameLine(200.0f);
                changed |= ImGui::Checkbox("Morton BVH", &settings.enableMortonBvhCulling);
                ImGui::Spacing(); ImGui::Separator(); ImGui::Spacing();

                changed |= ImGui::Checkbox("Global Frustum Culling", &settings.enableFrustumCulling);
                if (settings.enableFrustumCulling) {
                    ImGui::Indent(24.0f);
                    changed |= ImGui::Checkbox("Chunk Level##Frustum", &settings.enableChunkFrustumCulling);
                    ImGui::SameLine(200.0f);
                    changed |= ImGui::Checkbox("Model Level##Frustum", &settings.enableModelFrustumCulling);

                    changed |= ImGui::Checkbox("Mesh Level##Frustum", &settings.enableMeshFrustumCulling);
                    ImGui::SameLine(200.0f);
                    changed |= ImGui::Checkbox("Meshlet Level##Frustum", &settings.enableMeshletFrustumCulling);

                    changed |= ImGui::Checkbox("Point Lights##Frustum", &settings.enablePointLightFrustumCulling);
                    ImGui::SameLine(200.0f);
                    changed |= ImGui::Checkbox("Spot Lights##Frustum", &settings.enableSpotLightFrustumCulling);
                    ImGui::Unindent(24.0f);
                }
                ImGui::Spacing(); ImGui::Separator(); ImGui::Spacing();

                changed |= ImGui::Checkbox("Global Occlusion Culling (Hi-Z)", &settings.enableOcclusionCulling);
                if (settings.enableOcclusionCulling) {
                    ImGui::Indent(24.0f);
                    changed |= ImGui::Checkbox("Build Hi-Z Depth Pyramid", &settings.enableHiz);
                    ImGui::Spacing();

                    changed |= ImGui::Checkbox("Chunk Level##Occlusion", &settings.enableChunkOcclusionCulling);
                    ImGui::SameLine(200.0f);
                    changed |= ImGui::Checkbox("Model Level##Occlusion", &settings.enableModelOcclusionCulling);

                    changed |= ImGui::Checkbox("Mesh Level##Occlusion", &settings.enableMeshOcclusionCulling);
                    ImGui::SameLine(200.0f);
                    changed |= ImGui::Checkbox("Meshlet Level##Occlusion", &settings.enableMeshletOcclusionCulling);

                    changed |= ImGui::Checkbox("Point Lights##Occlusion", &settings.enablePointLightOcclusionCulling);
                    ImGui::SameLine(200.0f);
                    changed |= ImGui::Checkbox("Spot Lights##Occlusion", &settings.enableSpotLightOcclusionCulling);
                    ImGui::Unindent(24.0f);
                }
                ImGui::Spacing(); ImGui::Separator(); ImGui::Spacing();

                changed |= ImGui::Checkbox("GPU Geometry Culling", &settings.enableGeometryGpuCulling);
                if (settings.enableGeometryGpuCulling) {
                    ImGui::Indent(24.0f);
                    changed |= ImGui::Checkbox("Meshlet Cone Culling", &settings.enableMeshletConeCulling);
                    changed |= ImGui::Checkbox("Point Light Hardware Culling", &settings.enablePointLightGpuCulling);
                    changed |= ImGui::Checkbox("Spot Light Hardware Culling", &settings.enableSpotLightGpuCulling);
                    ImGui::Unindent(24.0f);
                }
            }
            Syn::UI::EndCard();

            constexpr const char* CardPostProcessingTitle = "Post-Processing & Effects";
            if (Syn::UI::BeginCard(CardPostProcessingTitle, SYN_ICON_MAGIC, getCardState(CardPostProcessingTitle))) {

                changed |= ImGui::Checkbox("Enable Bloom", &settings.enableBloom);
                if (settings.enableBloom) {
                    ImGui::Indent(24.0f);
                    changed |= ImGui::DragFloat("Threshold", &settings.bloomThreshold, 0.01f, 0.0f, 10.0f);
                    changed |= ImGui::DragFloat("Knee", &settings.bloomKnee, 0.01f, 0.0f, 1.0f);
                    changed |= ImGui::DragFloat("Filter Radius", &settings.bloomFilterRadius, 0.001f, 0.0f, 0.1f, "%.4f");
                    changed |= ImGui::DragFloat("Exposure", &settings.bloomExposure, 0.01f, 0.1f, 10.0f);
                    changed |= ImGui::DragFloat("Strength", &settings.bloomStrength, 0.01f, 0.0f, 5.0f);
                    ImGui::Unindent(24.0f);
                }
                ImGui::Spacing(); ImGui::Separator(); ImGui::Spacing();

                changed |= ImGui::Checkbox("Enable DP-HVO (SSAO)", &settings.enableSsao);
                if (settings.enableSsao) {
                    ImGui::Indent(24.0f);
                    changed |= ImGui::Checkbox("Apply to Lights", &settings.enableSsaoLight);
                    ImGui::Spacing();

                    changed |= ImGui::DragFloat("Radius", &settings.aoRadius, 0.01f, 0.0f, 100.f);
                    changed |= ImGui::DragFloat("Intensity", &settings.aoIntensity, 0.1f, 0.0f, 100.f);
                    changed |= ImGui::DragFloat("Max Distance", &settings.maxOcclusionDistance, 0.1f, 0.0f, 100.f);
                    changed |= ImGui::DragFloat("Depth Sharpness", &settings.depthSharpness, 0.05f, 0.0f, 100.f);
                    changed |= ImGui::DragFloat("Bias", &settings.bias, 0.001f, 0.0f, 100.f);
                    changed |= ImGui::DragInt("Sample Count", &settings.sampleCount, 1.0f, 1, 100);
                    ImGui::Unindent(24.0f);
                }
            }
            Syn::UI::EndCard();

            constexpr const char* CardLightingTitle = "Lighting Features";
            if (Syn::UI::BeginCard(CardLightingTitle, SYN_ICON_LIGHTBULB, getCardState(CardLightingTitle))) {

                if (settings.pipelineType == PipelineType::Deferred) {
                    ImGui::TextDisabled("Deferred Renderer Active");
                    changed |= ImGui::Checkbox("Emissive AO", &settings.enableDeferredEmissiveAo);
                    changed |= ImGui::Checkbox("Directional Lights", &settings.enableDeferredDirectionalLights);
                    changed |= ImGui::Checkbox("Point Lights", &settings.enableDeferredPointLights);
                    changed |= ImGui::Checkbox("Spot Lights", &settings.enableDeferredSpotLights);
                }
                else if (settings.pipelineType == PipelineType::ForwardPlus) {
                    ImGui::TextDisabled("Forward+ Renderer Active");
                    changed |= ImGui::Checkbox("Emissive AO", &settings.enableForwardPlusEmissiveAo);
                    changed |= ImGui::Checkbox("Directional Lights", &settings.enableForwardPlusDirectionalLights);
                    changed |= ImGui::Checkbox("Point Lights", &settings.enableForwardPlusPointLights);
                    changed |= ImGui::Checkbox("Spot Lights", &settings.enableForwardPlusSpotLights);
                }
            }
            Syn::UI::EndCard();

            constexpr const char* CardDebugTitle = "Debug & Visualization";
            if (Syn::UI::BeginCard(CardDebugTitle, SYN_ICON_BUG, getCardState(CardDebugTitle))) {

                changed |= ImGui::Checkbox("Enable Debug Camera", &settings.useDebugCamera);
                ImGui::Spacing(); ImGui::Separator(); ImGui::Spacing();

                ImGui::TextDisabled("Billboards");
                changed |= ImGui::Checkbox("Cameras", &settings.enableBillboardCameras);
                ImGui::SameLine(180.0f);
                changed |= ImGui::Checkbox("Directional Lights##BB", &settings.enableBillboardDirectionalLights);

                changed |= ImGui::Checkbox("Point Lights##BB", &settings.enableBillboardPointLights);
                ImGui::SameLine(180.0f);
                changed |= ImGui::Checkbox("Spot Lights##BB", &settings.enableBillboardSpotLights);

                ImGui::Spacing(); ImGui::Separator(); ImGui::Spacing();

                ImGui::TextDisabled("Light Wireframes");
                changed |= ImGui::Checkbox("Point Light Sphere", &settings.enablePointLightSphereWireframe);
                ImGui::SameLine(220.0f);
                changed |= ImGui::Checkbox("Point Light AABB", &settings.enablePointLightAabbWireframe);

                changed |= ImGui::Checkbox("Spot Light Sphere", &settings.enableSpotLightSphereWireframe);
                ImGui::SameLine(220.0f);
                changed |= ImGui::Checkbox("Spot Light AABB", &settings.enableSpotLightAabbWireframe);

                changed |= ImGui::Checkbox("Spot Light Cone", &settings.enableSpotLightConeWireframe);
                ImGui::SameLine(220.0f);
                changed |= ImGui::Checkbox("Spot Light Pyramid", &settings.enableSpotLightPyramidWireframe);

                ImGui::Spacing(); ImGui::Separator(); ImGui::Spacing();

                ImGui::TextDisabled("Geometry Wireframes");
                changed |= ImGui::Checkbox("Mesh AABB", &settings.enableWireframeMeshAabb);
                ImGui::SameLine(220.0f);
                changed |= ImGui::Checkbox("Mesh Sphere", &settings.enableWireframeMeshSphere);

                changed |= ImGui::Checkbox("Meshlet AABB", &settings.enableWireframeMeshletAabb);
                ImGui::SameLine(220.0f);
                changed |= ImGui::Checkbox("Meshlet Sphere", &settings.enableWireframeMeshletSphere);

                changed |= ImGui::Checkbox("Static Chunk AABB", &settings.enableStaticChunkAabbWireframe);
                ImGui::SameLine(220.0f);
                changed |= ImGui::Checkbox("Morton Chunk AABB", &settings.enableMortonChunkAabbWireframe);

                changed |= ImGui::Checkbox("Meshlet Cone", &settings.enableWireframeMeshletCone);

                ImGui::Spacing(); ImGui::Separator(); ImGui::Spacing();

                ImGui::TextDisabled("Physics Colliders");
                changed |= ImGui::Checkbox("Box Collider", &settings.enableBoxColliderWireframe);
                ImGui::SameLine(180.0f);
                changed |= ImGui::Checkbox("Sphere Collider", &settings.enableSphereColliderWireframe);
                changed |= ImGui::Checkbox("Capsule Collider", &settings.enableCapsuleColliderWireframe);
            }
            Syn::UI::EndCard();

            if (changed) {
                vm.Dispatch(UpdateSceneSettingsIntent{ settings });
            }

        }
        ImGui::End();
        ImGui::PopStyleVar();
    }
}