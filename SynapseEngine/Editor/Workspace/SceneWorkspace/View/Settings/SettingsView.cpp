#include "SettingsView.h"
#include "Editor/Manager/EditorIcons.h"
#include "Engine/Render/ComputeGroupSize.h"
#include "Editor/Widgets/CardWidget.h"
#include "Editor/Widgets/PropertyGrid.h"
#include "Editor/Widgets/Vector3Widget.h"
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

            constexpr const char* CardEnvironmentTitle = "Environment Settings";
            if (Syn::UI::BeginCard(CardEnvironmentTitle, SYN_ICON_SUN, getCardState(CardEnvironmentTitle))) {
                if (Syn::UI::BeginPropertyGrid("EnvironmentGrid")) {
                    drawSectionHeader("Skybox Render");

                    changed |= Syn::UI::PropertyCheckbox("Enable Sky", settings.environment.enableSky);

                    if (settings.environment.enableSky) {
                        const char* skyModes[] = { "None", "Equirectangular", "Octahedral", "Procedural" };
                        int currentMode = (int)settings.environment.skyMode;
                        Syn::UI::BeginProperty("Sky Mode");
                        if (ImGui::Combo("##SkyMode", &currentMode, skyModes, IM_ARRAYSIZE(skyModes))) {
                            settings.environment.skyMode = (SkyMode)currentMode;
                            changed = true;
                        }

                        if (settings.environment.skyMode == SkyMode::EquirectangularTexture || settings.environment.skyMode == SkyMode::OctahedralTexture) {

                            const auto& textures = state.availableSkyTextures;
                            std::vector<const char*> textureNames;
                            int currentTextureIdx = -1;

                            textureNames.push_back("None");
                            if (settings.environment.skyTextureId == UINT32_MAX) currentTextureIdx = 0;

                            for (int i = 0; i < textures.size(); ++i) {
                                textureNames.push_back(textures[i].second.c_str());
                                if (textures[i].first == settings.environment.skyTextureId) {
                                    currentTextureIdx = i + 1;
                                }
                            }

                            Syn::UI::BeginProperty("Sky Texture");
                            if (ImGui::Combo("##SkyTexture", &currentTextureIdx, textureNames.data(), textureNames.size())) {
                                if (currentTextureIdx == 0) {
                                    settings.environment.skyTextureId = UINT32_MAX;
                                }
                                else {
                                    settings.environment.skyTextureId = textures[currentTextureIdx - 1].first;
                                }
                                changed = true;
                            }
                        }

                        changed |= Syn::UI::PropertyDragFloat("Intensity", settings.environment.skyIntensity, 0.05f, 0.0f, 100.0f, "%.2f", 1);
                        changed |= Syn::UI::PropertyDragFloat("Exposure", settings.environment.skyExposureEV, 0.05f, -100.0f, 100.0f, "%.2f", 1);

                        Syn::UI::BeginProperty("Tint");
                        if (ImGui::ColorEdit3("##SkyTint", glm::value_ptr(settings.environment.skyTint), ImGuiColorEditFlags_NoInputs)) {
                            changed = true;
                        }

                        Syn::UI::BeginProperty("Rotation");
                        bool rotDeactivated = false;
                        if (Syn::UI::DrawVec3Control("SkyRot", settings.environment.skyRotation, 0.0f, rotDeactivated)) {
                            changed = true;
                        }
                    }

                    Syn::UI::PropertySeparator();
                    drawSectionHeader("Fog & Atmospherics");
                    changed |= Syn::UI::PropertyCheckbox("Enable Fog", settings.environment.enableFog);
                    if (settings.environment.enableFog) {
                        Syn::UI::BeginProperty("Fog Color");
                        if (ImGui::ColorEdit3("##FogColor", glm::value_ptr(settings.environment.fogColor), ImGuiColorEditFlags_NoInputs)) {
                            changed = true;
                        }
                        changed |= Syn::UI::PropertyDragFloat("Fog Density", settings.environment.fogDensity, 0.001f, 0.0f, 1.0f, "%.4f", 1);
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
                    changed |= Syn::UI::PropertyCheckbox("Physics Debug", settings.debug.enablePhysicsWireframe);

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

            constexpr const char* CardGridTitle = "Infinite Grid & Axes";
            if (Syn::UI::BeginCard(CardGridTitle, SYN_ICON_TH, getCardState(CardGridTitle))) {

                if (Syn::UI::BeginPropertyGrid("InfiniteGridGrid")) {

                    changed |= Syn::UI::PropertyCheckbox("Enable Infinite Grid", settings.debug.enableInfiniteGrid);

                    if (settings.debug.enableInfiniteGrid) {
                        changed |= Syn::UI::PropertyDragFloat("Grid Scale", settings.debug.gridScale, 0.1f, 0.1f, 100.0f, "%.1f", 1);
                        changed |= Syn::UI::PropertyDragFloat("Fade Distance", settings.debug.fadeDistance, 1.0f, 10.0f, 1000.0f, "%.1f", 1);

                        // Új vastagság beállítások
                        changed |= Syn::UI::PropertyDragFloat("Grid Line Thickness", settings.debug.gridThickness, 0.05f, 0.1f, 10.0f, "%.2f", 1);
                        changed |= Syn::UI::PropertyDragFloat("Axis Line Thickness", settings.debug.axisThickness, 0.05f, 0.1f, 10.0f, "%.2f", 1);

                        Syn::UI::PropertySeparator();

                        drawSectionHeader("Grid Planes & Main Color");

                        Syn::UI::BeginProperty("Grid Color");
                        if (ImGui::ColorEdit4("##GridColorPicker", glm::value_ptr(settings.debug.gridColor), ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_AlphaPreview)) {
                            changed = true;
                        }

                        changed |= Syn::UI::PropertyCheckbox("Show XZ Plane (Floor)", settings.debug.gridShowXZ);
                        changed |= Syn::UI::PropertyCheckbox("Show XY Plane (Wall)", settings.debug.gridShowXY);
                        changed |= Syn::UI::PropertyCheckbox("Show YZ Plane (Wall)", settings.debug.gridShowYZ);

                        Syn::UI::PropertySeparator();

                        drawSectionHeader("World Axes & Custom Colors");

                        changed |= Syn::UI::PropertyCheckbox("Show X Axis", settings.debug.gridShowAxisX);
                        ImGui::SameLine();
                        if (ImGui::ColorEdit4("##AxisXColorPicker", glm::value_ptr(settings.debug.axisXColor), ImGuiColorEditFlags_NoInputs)) {
                            changed = true;
                        }

                        changed |= Syn::UI::PropertyCheckbox("Show Y Axis", settings.debug.gridShowAxisY);
                        ImGui::SameLine();
                        if (ImGui::ColorEdit4("##AxisYColorPicker", glm::value_ptr(settings.debug.axisYColor), ImGuiColorEditFlags_NoInputs)) {
                            changed = true;
                        }

                        changed |= Syn::UI::PropertyCheckbox("Show Z Axis", settings.debug.gridShowAxisZ);
                        ImGui::SameLine();
                        if (ImGui::ColorEdit4("##AxisZColorPicker", glm::value_ptr(settings.debug.axisZColor), ImGuiColorEditFlags_NoInputs)) {
                            changed = true;
                        }
                    }

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