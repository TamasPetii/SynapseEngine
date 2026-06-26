#include "SettingsView.h"
#include "Editor/Manager/EditorIcons.h"
#include "Engine/Render/ComputeGroupSize.h"
#include "Editor/Widgets/CardWidget.h"
#include "Editor/Widgets/PropertyGrid.h"
#include <imgui.h>
#include <string>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

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
                    int currentPipeline = (int)settings.lighting.pipelineType;

                    Syn::UI::BeginProperty("Pipeline Architecture");
                    if (ImGui::Combo("##Pipeline", &currentPipeline, pipelineNames, IM_ARRAYSIZE(pipelineNames))) {
                        settings.lighting.pipelineType = (PipelineType)currentPipeline;
                        changed = true;
                    }

                    if (settings.lighting.pipelineType == PipelineType::ForwardPlus) {
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
                            if (settings.lighting.tileSize == sizes[i]) {
                                currentTileSizeIndex = i; break;
                            }
                        }

                        Syn::UI::BeginProperty("Compute Tile Size");
                        if (ImGui::Combo("##TileSize", &currentTileSizeIndex, sliderNames, IM_ARRAYSIZE(sliderNames))) {
                            settings.lighting.tileSize = sizes[currentTileSizeIndex];
                            changed = true;
                        }
                    }

                    Syn::UI::PropertySeparator();
                    changed |= Syn::UI::PropertySliderFloat("Ambient Strength", settings.lighting.ambientStrength, 0.0f, 1.0f);
                    changed |= Syn::UI::PropertySliderFloat("Emissive Strength", settings.lighting.emissiveStrength, 0.0f, 10.0f);

                    Syn::UI::EndPropertyGrid();
                }
            }
            Syn::UI::EndCard();

            constexpr const char* CardCullingTitle = "Culling & Optimization";
            if (Syn::UI::BeginCard(CardCullingTitle, SYN_ICON_CROP, getCardState(CardCullingTitle))) {

                if (Syn::UI::BeginPropertyGrid("CullingGrid")) {

                    drawSectionHeader("Hardware Culling Devices");

                    const char* deviceNames[] = { SYN_ICON_MICROCHIP " CPU", SYN_ICON_DESKTOP " GPU" };

                    auto DrawDeviceProperty = [&](const char* label, CullingDeviceType& device) {
                        int currentDevice = (int)device;
                        Syn::UI::BeginProperty(label);
                        if (ImGui::Combo(std::string("##" + std::string(label)).c_str(), &currentDevice, deviceNames, IM_ARRAYSIZE(deviceNames))) {
                            device = (CullingDeviceType)currentDevice;
                            return true;
                        }
                        return false;
                        };

                    changed |= DrawDeviceProperty("Main Geometry Culling", settings.culling.geometryCullingDevice);
                    changed |= DrawDeviceProperty("DirLight Shadow Culling", settings.culling.directionLightShadowCullingDevice);
                    changed |= DrawDeviceProperty("Point Light Culling", settings.culling.pointLightCullingDevice);
                    changed |= DrawDeviceProperty("PointLight Shadow Culling", settings.culling.pointLightShadowCullingDevice);
                    changed |= DrawDeviceProperty("Spot Light Culling", settings.culling.spotLightCullingDevice);
                    changed |= DrawDeviceProperty("SpotLight Shadow Culling", settings.culling.spotLightShadowCullingDevice);

                    Syn::UI::PropertySeparator();

                    drawSectionHeader("Spatial Acceleration Structures");

                    const char* spatialAccNames[] = { "None", "Static BVH", "Morton BVH" };
                    auto DrawSpatialAccProperty = [&](const char* label, SpatialAccelerationType& type) {
                        int currentType = (int)type;
                        Syn::UI::BeginProperty(label);
                        if (ImGui::Combo(std::string("##" + std::string(label)).c_str(), &currentType, spatialAccNames, IM_ARRAYSIZE(spatialAccNames))) {
                            type = (SpatialAccelerationType)currentType;
                            return true;
                        }
                        return false;
                        };

                    changed |= DrawSpatialAccProperty("Main Geometry", settings.culling.geometrySpatialAcceleration);
                    changed |= DrawSpatialAccProperty("DirLight Shadow", settings.culling.directionLightShadowSpatialAcceleration);
                    changed |= DrawSpatialAccProperty("SpotLight Shadow", settings.culling.spotLightShadowSpatialAcceleration);
                    changed |= DrawSpatialAccProperty("PointLight Shadow", settings.culling.pointLightShadowSpatialAcceleration);

                    Syn::UI::PropertySeparator();

                    changed |= Syn::UI::PropertyCheckbox("Global Frustum Culling", settings.culling.enableFrustumCulling);
                    if (settings.culling.enableFrustumCulling) {
                        changed |= Syn::UI::PropertyCheckbox("Chunk Level##Frustum", settings.culling.enableChunkFrustumCulling, 1);
                        changed |= Syn::UI::PropertyCheckbox("Model Level##Frustum", settings.culling.enableModelFrustumCulling, 1);
                        changed |= Syn::UI::PropertyCheckbox("Mesh Level##Frustum", settings.culling.enableMeshFrustumCulling, 1);
                        changed |= Syn::UI::PropertyCheckbox("Meshlet Level##Frustum", settings.culling.enableMeshletFrustumCulling, 1);
                        changed |= Syn::UI::PropertyCheckbox("Point Lights##Frustum", settings.culling.enablePointLightFrustumCulling, 1);
                        changed |= Syn::UI::PropertyCheckbox("Spot Lights##Frustum", settings.culling.enableSpotLightFrustumCulling, 1);
                    }

                    Syn::UI::PropertySeparator();

                    changed |= Syn::UI::PropertyCheckbox("Global Occlusion Culling (Hi-Z)", settings.culling.enableOcclusionCulling);
                    if (settings.culling.enableOcclusionCulling) {
                        changed |= Syn::UI::PropertyCheckbox("Build Hi-Z Depth Pyramid", settings.culling.enableHiz, 1);
                        changed |= Syn::UI::PropertyCheckbox("Chunk Level##Occlusion", settings.culling.enableChunkOcclusionCulling, 1);
                        changed |= Syn::UI::PropertyCheckbox("Model Level##Occlusion", settings.culling.enableModelOcclusionCulling, 1);
                        changed |= Syn::UI::PropertyCheckbox("Mesh Level##Occlusion", settings.culling.enableMeshOcclusionCulling, 1);
                        changed |= Syn::UI::PropertyCheckbox("Meshlet Level##Occlusion", settings.culling.enableMeshletOcclusionCulling, 1);
                        changed |= Syn::UI::PropertyCheckbox("Point Lights##Occlusion", settings.culling.enablePointLightOcclusionCulling, 1);
                        changed |= Syn::UI::PropertyCheckbox("Spot Lights##Occlusion", settings.culling.enableSpotLightOcclusionCulling, 1);
                    }

                    Syn::UI::PropertySeparator();

                    changed |= Syn::UI::PropertyCheckbox("Meshlet Cone Culling", settings.culling.enableMeshletConeCulling);

                    Syn::UI::EndPropertyGrid();
                }
            }
            Syn::UI::EndCard();

            constexpr const char* CardPostProcessingTitle = "Post-Processing & Effects";
            if (Syn::UI::BeginCard(CardPostProcessingTitle, SYN_ICON_MAGIC, getCardState(CardPostProcessingTitle))) {

                if (Syn::UI::BeginPropertyGrid("PostProcessGrid")) {
                    changed |= Syn::UI::PropertyCheckbox("Enable Bloom", settings.postProcess.enableBloom);
                    if (settings.postProcess.enableBloom) {
                        changed |= Syn::UI::PropertyDragFloat("Threshold", settings.postProcess.bloomThreshold, 0.01f, 0.0f, 10.0f, "%.3f", 1);
                        changed |= Syn::UI::PropertyDragFloat("Knee", settings.postProcess.bloomKnee, 0.01f, 0.0f, 1.0f, "%.3f", 1);
                        changed |= Syn::UI::PropertyDragFloat("Filter Radius", settings.postProcess.bloomFilterRadius, 0.001f, 0.0f, 0.1f, "%.4f", 1);
                        changed |= Syn::UI::PropertyDragFloat("Exposure", settings.postProcess.bloomExposure, 0.01f, 0.1f, 10.0f, "%.3f", 1);
                        changed |= Syn::UI::PropertyDragFloat("Strength", settings.postProcess.bloomStrength, 0.01f, 0.0f, 5.0f, "%.3f", 1);
                    }

                    Syn::UI::PropertySeparator();

                    changed |= Syn::UI::PropertyCheckbox("Enable DP-HVO (SSAO)", settings.postProcess.enableSsao);
                    if (settings.postProcess.enableSsao) {
                        changed |= Syn::UI::PropertyCheckbox("Apply to Lights", settings.postProcess.enableSsaoLight, 1);
                        changed |= Syn::UI::PropertyDragFloat("Radius", settings.postProcess.aoRadius, 0.01f, 0.0f, 100.f, "%.3f", 1);
                        changed |= Syn::UI::PropertyDragFloat("Intensity", settings.postProcess.aoIntensity, 0.1f, 0.0f, 100.f, "%.3f", 1);
                        changed |= Syn::UI::PropertyDragFloat("Max Distance", settings.postProcess.maxOcclusionDistance, 0.1f, 0.0f, 100.f, "%.3f", 1);
                        changed |= Syn::UI::PropertyDragFloat("Depth Sharpness", settings.postProcess.depthSharpness, 0.05f, 0.0f, 100.f, "%.3f", 1);
                        changed |= Syn::UI::PropertyDragFloat("Bias", settings.postProcess.bias, 0.001f, 0.0f, 100.f, "%.4f", 1);

                        Syn::UI::BeginProperty("Sample Count", 1);
                        changed |= ImGui::DragInt("##SampleCount", &settings.postProcess.sampleCount, 1.0f, 1, 100);
                    }

                    Syn::UI::EndPropertyGrid();
                }
            }
            Syn::UI::EndCard();

            constexpr const char* CardLightingTitle = "Lighting Features";
            if (Syn::UI::BeginCard(CardLightingTitle, SYN_ICON_LIGHTBULB, getCardState(CardLightingTitle))) {

                if (Syn::UI::BeginPropertyGrid("LightingFeaturesGrid")) {
                    if (settings.lighting.pipelineType == PipelineType::Deferred) {
                        drawSectionHeader("Deferred Renderer Active");
                        changed |= Syn::UI::PropertyCheckbox("Emissive AO", settings.lighting.enableDeferredEmissiveAo);
                        changed |= Syn::UI::PropertyCheckbox("Directional Lights", settings.lighting.enableDeferredDirectionalLights);
                        changed |= Syn::UI::PropertyCheckbox("Point Lights", settings.lighting.enableDeferredPointLights);
                        changed |= Syn::UI::PropertyCheckbox("Spot Lights", settings.lighting.enableDeferredSpotLights);
                    }
                    else if (settings.lighting.pipelineType == PipelineType::ForwardPlus) {
                        drawSectionHeader("Forward+ Renderer Active");
                        changed |= Syn::UI::PropertyCheckbox("Emissive AO", settings.lighting.enableForwardPlusEmissiveAo);
                        changed |= Syn::UI::PropertyCheckbox("Directional Lights", settings.lighting.enableForwardPlusDirectionalLights);
                        changed |= Syn::UI::PropertyCheckbox("Point Lights", settings.lighting.enableForwardPlusPointLights);
                        changed |= Syn::UI::PropertyCheckbox("Spot Lights", settings.lighting.enableForwardPlusSpotLights);
                    }

                    Syn::UI::EndPropertyGrid();
                }
            }
            Syn::UI::EndCard();

            constexpr const char* CardDebugTitle = "Debug & Visualization";
            if (Syn::UI::BeginCard(CardDebugTitle, SYN_ICON_BUG, getCardState(CardDebugTitle))) {

                if (Syn::UI::BeginPropertyGrid("DebugGrid")) {
                    changed |= Syn::UI::PropertyCheckbox("Enable Debug Camera", settings.debug.useDebugCamera);

                    Syn::UI::PropertySeparator();

                    drawSectionHeader("Billboards");
                    changed |= Syn::UI::PropertyCheckbox("Cameras", settings.debug.enableBillboardCameras);
                    changed |= Syn::UI::PropertyCheckbox("Directional Lights", settings.debug.enableBillboardDirectionalLights);
                    changed |= Syn::UI::PropertyCheckbox("Point Lights", settings.debug.enableBillboardPointLights);
                    changed |= Syn::UI::PropertyCheckbox("Spot Lights", settings.debug.enableBillboardSpotLights);

                    Syn::UI::PropertySeparator();

                    drawSectionHeader("Light Wireframes");
                    changed |= Syn::UI::PropertyCheckbox("Point Light Sphere", settings.debug.enablePointLightSphereWireframe);
                    changed |= Syn::UI::PropertyCheckbox("Point Light AABB", settings.debug.enablePointLightAabbWireframe);
                    changed |= Syn::UI::PropertyCheckbox("Spot Light Sphere", settings.debug.enableSpotLightSphereWireframe);
                    changed |= Syn::UI::PropertyCheckbox("Spot Light AABB", settings.debug.enableSpotLightAabbWireframe);
                    changed |= Syn::UI::PropertyCheckbox("Spot Light Cone", settings.debug.enableSpotLightConeWireframe);
                    changed |= Syn::UI::PropertyCheckbox("Spot Light Pyramid", settings.debug.enableSpotLightPyramidWireframe);

                    Syn::UI::PropertySeparator();

                    drawSectionHeader("Geometry Wireframes");
                    changed |= Syn::UI::PropertyCheckbox("Mesh AABB", settings.debug.enableWireframeMeshAabb);
                    changed |= Syn::UI::PropertyCheckbox("Mesh Sphere", settings.debug.enableWireframeMeshSphere);
                    changed |= Syn::UI::PropertyCheckbox("Meshlet AABB", settings.debug.enableWireframeMeshletAabb);
                    changed |= Syn::UI::PropertyCheckbox("Meshlet Sphere", settings.debug.enableWireframeMeshletSphere);
                    changed |= Syn::UI::PropertyCheckbox("Static Chunk AABB", settings.debug.enableStaticChunkAabbWireframe);
                    changed |= Syn::UI::PropertyCheckbox("Morton Chunk AABB", settings.debug.enableMortonChunkAabbWireframe);
                    changed |= Syn::UI::PropertyCheckbox("Meshlet Cone", settings.debug.enableWireframeMeshletCone);

                    Syn::UI::PropertySeparator();

                    drawSectionHeader("Physics Colliders");
                    changed |= Syn::UI::PropertyCheckbox("Box Collider", settings.debug.enableBoxColliderWireframe);
                    changed |= Syn::UI::PropertyCheckbox("Sphere Collider", settings.debug.enableSphereColliderWireframe);
                    changed |= Syn::UI::PropertyCheckbox("Capsule Collider", settings.debug.enableCapsuleColliderWireframe);

                    Syn::UI::PropertySeparator();

                    drawSectionHeader("Editor Selection Outlines");
                    changed |= Syn::UI::PropertyCheckbox("Selected Entity Outline", settings.debug.enableSelectedOutline);
                    changed |= Syn::UI::PropertyCheckbox("Hierarchy Outline", settings.debug.enableSelectedHierarchyOutline);

                    Syn::UI::BeginProperty("Primary Color");
                    if (ImGui::ColorEdit4("##OutlinePrimary", glm::value_ptr(settings.debug.outlinePrimaryColor), ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_AlphaPreview)) {
                        changed = true;
                    }

                    Syn::UI::BeginProperty("Secondary Color");
                    if (ImGui::ColorEdit4("##OutlineSecondary", glm::value_ptr(settings.debug.outlineSecondaryColor), ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_AlphaPreview)) {
                        changed = true;
                    }

                    changed |= Syn::UI::PropertyDragFloat("Thickness", settings.debug.outlineThickness, 0.1f, 1.0f, 10.0f, "%.1f", 1);

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