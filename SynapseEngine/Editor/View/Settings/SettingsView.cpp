#include "SettingsView.h"
#include "Editor/Manager/EditorIcons.h"
#include "Engine/Render/ComputeGroupSize.h"
#include "Editor/Widgets/CardWidget.h"
#include "Editor/Widgets/PropertyGrid.h"
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

            auto drawSectionHeader = [](const char* title) {
                ImGui::TableNextRow();
                ImGui::TableSetColumnIndex(0);
                ImGui::Spacing();
                ImGui::TextDisabled("%s", title);
                };

            constexpr const char* CardGlobalPipelineTitle = "Global Pipeline";
            if (Syn::UI::BeginCard(CardGlobalPipelineTitle, SYN_ICON_GLOBE, getCardState(CardGlobalPipelineTitle))) {

                if (Syn::UI::BeginPropertyGrid("GlobalPipelineGrid")) {
                    const char* pipelineNames[] = { "Deferred", "Forward+" };
                    int currentPipeline = (int)settings.pipelineType;

                    Syn::UI::BeginProperty("Pipeline Architecture");
                    if (ImGui::Combo("##Pipeline", &currentPipeline, pipelineNames, IM_ARRAYSIZE(pipelineNames))) {
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

                        Syn::UI::BeginProperty("Compute Tile Size");
                        if (ImGui::Combo("##TileSize", &currentTileSizeIndex, sliderNames, IM_ARRAYSIZE(sliderNames))) {
                            settings.tileSize = sizes[currentTileSizeIndex];
                            changed = true;
                        }
                    }

                    Syn::UI::PropertySeparator();
                    changed |= Syn::UI::PropertySliderFloat("Ambient Strength", settings.ambientStrength, 0.0f, 1.0f);
                    changed |= Syn::UI::PropertySliderFloat("Emissive Strength", settings.emissiveStrength, 0.0f, 10.0f);

                    Syn::UI::EndPropertyGrid();
                }
            }
            Syn::UI::EndCard();

            constexpr const char* CardCullingTitle = "Culling & Optimization";
            if (Syn::UI::BeginCard(CardCullingTitle, SYN_ICON_CROP, getCardState(CardCullingTitle))) {

                if (Syn::UI::BeginPropertyGrid("CullingGrid")) {

                    drawSectionHeader("Spatial Acceleration");
                    changed |= Syn::UI::PropertyCheckbox("Static BVH", settings.enableStaticBvhCulling);
                    changed |= Syn::UI::PropertyCheckbox("Morton BVH", settings.enableMortonBvhCulling);

                    Syn::UI::PropertySeparator();

                    changed |= Syn::UI::PropertyCheckbox("Global Frustum Culling", settings.enableFrustumCulling);
                    if (settings.enableFrustumCulling) {
                        changed |= Syn::UI::PropertyCheckbox("Chunk Level", settings.enableChunkFrustumCulling, 1);
                        changed |= Syn::UI::PropertyCheckbox("Model Level", settings.enableModelFrustumCulling, 1);
                        changed |= Syn::UI::PropertyCheckbox("Mesh Level", settings.enableMeshFrustumCulling, 1);
                        changed |= Syn::UI::PropertyCheckbox("Meshlet Level", settings.enableMeshletFrustumCulling, 1);
                        changed |= Syn::UI::PropertyCheckbox("Point Lights", settings.enablePointLightFrustumCulling, 1);
                        changed |= Syn::UI::PropertyCheckbox("Spot Lights", settings.enableSpotLightFrustumCulling, 1);
                    }

                    Syn::UI::PropertySeparator();

                    changed |= Syn::UI::PropertyCheckbox("Global Occlusion Culling (Hi-Z)", settings.enableOcclusionCulling);
                    if (settings.enableOcclusionCulling) {
                        changed |= Syn::UI::PropertyCheckbox("Build Hi-Z Depth Pyramid", settings.enableHiz, 1);
                        changed |= Syn::UI::PropertyCheckbox("Chunk Level", settings.enableChunkOcclusionCulling, 1);
                        changed |= Syn::UI::PropertyCheckbox("Model Level", settings.enableModelOcclusionCulling, 1);
                        changed |= Syn::UI::PropertyCheckbox("Mesh Level", settings.enableMeshOcclusionCulling, 1);
                        changed |= Syn::UI::PropertyCheckbox("Meshlet Level", settings.enableMeshletOcclusionCulling, 1);
                        changed |= Syn::UI::PropertyCheckbox("Point Lights", settings.enablePointLightOcclusionCulling, 1);
                        changed |= Syn::UI::PropertyCheckbox("Spot Lights", settings.enableSpotLightOcclusionCulling, 1);
                    }

                    Syn::UI::PropertySeparator();

                    changed |= Syn::UI::PropertyCheckbox("GPU Geometry Culling", settings.enableGeometryGpuCulling);
                    if (settings.enableGeometryGpuCulling) {
                        changed |= Syn::UI::PropertyCheckbox("Meshlet Cone Culling", settings.enableMeshletConeCulling, 1);
                        changed |= Syn::UI::PropertyCheckbox("Point Light Hardware Culling", settings.enablePointLightGpuCulling, 1);
                        changed |= Syn::UI::PropertyCheckbox("Spot Light Hardware Culling", settings.enableSpotLightGpuCulling, 1);
                    }

                    Syn::UI::EndPropertyGrid();
                }
            }
            Syn::UI::EndCard();

            constexpr const char* CardPostProcessingTitle = "Post-Processing & Effects";
            if (Syn::UI::BeginCard(CardPostProcessingTitle, SYN_ICON_MAGIC, getCardState(CardPostProcessingTitle))) {

                if (Syn::UI::BeginPropertyGrid("PostProcessGrid")) {
                    changed |= Syn::UI::PropertyCheckbox("Enable Bloom", settings.enableBloom);
                    if (settings.enableBloom) {
                        changed |= Syn::UI::PropertyDragFloat("Threshold", settings.bloomThreshold, 0.01f, 0.0f, 10.0f, "%.3f", 1);
                        changed |= Syn::UI::PropertyDragFloat("Knee", settings.bloomKnee, 0.01f, 0.0f, 1.0f, "%.3f", 1);
                        changed |= Syn::UI::PropertyDragFloat("Filter Radius", settings.bloomFilterRadius, 0.001f, 0.0f, 0.1f, "%.4f", 1);
                        changed |= Syn::UI::PropertyDragFloat("Exposure", settings.bloomExposure, 0.01f, 0.1f, 10.0f, "%.3f", 1);
                        changed |= Syn::UI::PropertyDragFloat("Strength", settings.bloomStrength, 0.01f, 0.0f, 5.0f, "%.3f", 1);
                    }

                    Syn::UI::PropertySeparator();

                    changed |= Syn::UI::PropertyCheckbox("Enable DP-HVO (SSAO)", settings.enableSsao);
                    if (settings.enableSsao) {
                        changed |= Syn::UI::PropertyCheckbox("Apply to Lights", settings.enableSsaoLight, 1);
                        changed |= Syn::UI::PropertyDragFloat("Radius", settings.aoRadius, 0.01f, 0.0f, 100.f, "%.3f", 1);
                        changed |= Syn::UI::PropertyDragFloat("Intensity", settings.aoIntensity, 0.1f, 0.0f, 100.f, "%.3f", 1);
                        changed |= Syn::UI::PropertyDragFloat("Max Distance", settings.maxOcclusionDistance, 0.1f, 0.0f, 100.f, "%.3f", 1);
                        changed |= Syn::UI::PropertyDragFloat("Depth Sharpness", settings.depthSharpness, 0.05f, 0.0f, 100.f, "%.3f", 1);
                        changed |= Syn::UI::PropertyDragFloat("Bias", settings.bias, 0.001f, 0.0f, 100.f, "%.4f", 1);

                        Syn::UI::BeginProperty("Sample Count", 1);
                        changed |= ImGui::DragInt("##SampleCount", &settings.sampleCount, 1.0f, 1, 100);
                    }

                    Syn::UI::EndPropertyGrid();
                }
            }
            Syn::UI::EndCard();

            constexpr const char* CardLightingTitle = "Lighting Features";
            if (Syn::UI::BeginCard(CardLightingTitle, SYN_ICON_LIGHTBULB, getCardState(CardLightingTitle))) {

                if (Syn::UI::BeginPropertyGrid("LightingFeaturesGrid")) {
                    if (settings.pipelineType == PipelineType::Deferred) {
                        drawSectionHeader("Deferred Renderer Active");
                        changed |= Syn::UI::PropertyCheckbox("Emissive AO", settings.enableDeferredEmissiveAo);
                        changed |= Syn::UI::PropertyCheckbox("Directional Lights", settings.enableDeferredDirectionalLights);
                        changed |= Syn::UI::PropertyCheckbox("Point Lights", settings.enableDeferredPointLights);
                        changed |= Syn::UI::PropertyCheckbox("Spot Lights", settings.enableDeferredSpotLights);
                    }
                    else if (settings.pipelineType == PipelineType::ForwardPlus) {
                        drawSectionHeader("Forward+ Renderer Active");
                        changed |= Syn::UI::PropertyCheckbox("Emissive AO", settings.enableForwardPlusEmissiveAo);
                        changed |= Syn::UI::PropertyCheckbox("Directional Lights", settings.enableForwardPlusDirectionalLights);
                        changed |= Syn::UI::PropertyCheckbox("Point Lights", settings.enableForwardPlusPointLights);
                        changed |= Syn::UI::PropertyCheckbox("Spot Lights", settings.enableForwardPlusSpotLights);
                    }

                    Syn::UI::EndPropertyGrid();
                }
            }
            Syn::UI::EndCard();

            constexpr const char* CardDebugTitle = "Debug & Visualization";
            if (Syn::UI::BeginCard(CardDebugTitle, SYN_ICON_BUG, getCardState(CardDebugTitle))) {

                if (Syn::UI::BeginPropertyGrid("DebugGrid")) {
                    changed |= Syn::UI::PropertyCheckbox("Enable Debug Camera", settings.useDebugCamera);

                    Syn::UI::PropertySeparator();

                    drawSectionHeader("Billboards");
                    changed |= Syn::UI::PropertyCheckbox("Cameras", settings.enableBillboardCameras);
                    changed |= Syn::UI::PropertyCheckbox("Directional Lights", settings.enableBillboardDirectionalLights);
                    changed |= Syn::UI::PropertyCheckbox("Point Lights", settings.enableBillboardPointLights);
                    changed |= Syn::UI::PropertyCheckbox("Spot Lights", settings.enableBillboardSpotLights);

                    Syn::UI::PropertySeparator();

                    drawSectionHeader("Light Wireframes");
                    changed |= Syn::UI::PropertyCheckbox("Point Light Sphere", settings.enablePointLightSphereWireframe);
                    changed |= Syn::UI::PropertyCheckbox("Point Light AABB", settings.enablePointLightAabbWireframe);
                    changed |= Syn::UI::PropertyCheckbox("Spot Light Sphere", settings.enableSpotLightSphereWireframe);
                    changed |= Syn::UI::PropertyCheckbox("Spot Light AABB", settings.enableSpotLightAabbWireframe);
                    changed |= Syn::UI::PropertyCheckbox("Spot Light Cone", settings.enableSpotLightConeWireframe);
                    changed |= Syn::UI::PropertyCheckbox("Spot Light Pyramid", settings.enableSpotLightPyramidWireframe);

                    Syn::UI::PropertySeparator();

                    drawSectionHeader("Geometry Wireframes");
                    changed |= Syn::UI::PropertyCheckbox("Mesh AABB", settings.enableWireframeMeshAabb);
                    changed |= Syn::UI::PropertyCheckbox("Mesh Sphere", settings.enableWireframeMeshSphere);
                    changed |= Syn::UI::PropertyCheckbox("Meshlet AABB", settings.enableWireframeMeshletAabb);
                    changed |= Syn::UI::PropertyCheckbox("Meshlet Sphere", settings.enableWireframeMeshletSphere);
                    changed |= Syn::UI::PropertyCheckbox("Static Chunk AABB", settings.enableStaticChunkAabbWireframe);
                    changed |= Syn::UI::PropertyCheckbox("Morton Chunk AABB", settings.enableMortonChunkAabbWireframe);
                    changed |= Syn::UI::PropertyCheckbox("Meshlet Cone", settings.enableWireframeMeshletCone);

                    Syn::UI::PropertySeparator();

                    drawSectionHeader("Physics Colliders");
                    changed |= Syn::UI::PropertyCheckbox("Box Collider", settings.enableBoxColliderWireframe);
                    changed |= Syn::UI::PropertyCheckbox("Sphere Collider", settings.enableSphereColliderWireframe);
                    changed |= Syn::UI::PropertyCheckbox("Capsule Collider", settings.enableCapsuleColliderWireframe);

                    Syn::UI::EndPropertyGrid();
                }
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