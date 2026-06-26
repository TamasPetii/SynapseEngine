#include "ViewportView.h"
#include "Engine/Vk/Image/ImageUtils.h"
#include "Editor/Manager/EditorIcons.h"
#include <ImGuizmo.h>
#include <imgui.h>
#include <imgui_internal.h>
#include <glm/gtc/type_ptr.hpp>
#include "Engine/Scene/DrawData/SceneDrawData.h"

namespace Syn {

    void ViewportView::Draw(ViewportViewModel& vm) {
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2{ 0, 0 });

        ImGui::Begin(SYN_ICON_GAMEPAD " Viewport", nullptr);

        ViewportState state = vm.GetState();

        ImVec2 viewportPanelSize = ImGui::GetContentRegionAvail();
        uint32_t currentWidth = static_cast<uint32_t>(viewportPanelSize.x);
        uint32_t currentHeight = static_cast<uint32_t>(viewportPanelSize.y);

        bool isResizing = (currentWidth > 0 && currentHeight > 0 && (currentWidth != state.width || currentHeight != state.height));

        vm.Dispatch(ResizeViewportIntent{ currentWidth, currentHeight });

        ImVec2 imageStartPos = ImGui::GetCursorScreenPos();

        if (viewportPanelSize.x <= 0.0f) viewportPanelSize.x = 1.0f;
        if (viewportPanelSize.y <= 0.0f) viewportPanelSize.y = 1.0f;

        if (state.textureId && !isResizing) {
            ImGui::Image(state.textureId, viewportPanelSize);
        }
        else {
            ImGui::Dummy(viewportPanelSize);
        }

        ImVec2 vMin = ImGui::GetItemRectMin();
        ImVec2 vMax = ImGui::GetItemRectMax();
        bool isImageHovered = ImGui::IsItemHovered();

        RenderFloatingToolbar(vm, state, imageStartPos, viewportPanelSize);
		RenderSimulationToolbar(vm, state, imageStartPos, viewportPanelSize);

        if (isImageHovered && ImGui::IsMouseClicked(ImGuiMouseButton_Left) && !ImGuizmo::IsOver() && !ImGui::IsAnyItemHovered()) {
            ImVec2 mousePos = ImGui::GetMousePos();
            uint32_t x = static_cast<uint32_t>(mousePos.x - vMin.x);
            uint32_t y = static_cast<uint32_t>(mousePos.y - vMin.y);
            vm.Dispatch(PickEntityIntent{ x, y });
        }

        DrawGizmo(vm, state, imageStartPos, viewportPanelSize);
        HandleShortcuts(vm);

        ImGui::End();
        ImGui::PopStyleVar();
    }

    void ViewportView::RenderSimulationToolbar(ViewportViewModel& vm, const ViewportState& state, ImVec2 startPos, ImVec2 size) {
        float toolbarWidth = 110.0f;
        float toolbarHeight = 40.0f;

        ImGui::SetCursorScreenPos(ImVec2(startPos.x + (size.x - toolbarWidth) * 0.5f, startPos.y + 8.0f));

        ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.1f, 0.1f, 0.1f, 0.85f));
        ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, 6.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(8.0f, 6.0f));

        if (ImGui::BeginChild("##SimulationToolbar", ImVec2(toolbarWidth, toolbarHeight), false, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_AlwaysUseWindowPadding)) {

            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 0, 0, 0));
            ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(4.0f, 6.0f));

            if (state.simState == SimulationState::Playing)
                ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.3f, 0.9f, 0.3f, 1.0f));

            if (ImGui::Button(SYN_ICON_PLAY, ImVec2(26, 28))) 
                vm.Dispatch(PlaySimulationIntent{});

            if (state.simState == SimulationState::Playing) 
                ImGui::PopStyleColor();

            if (ImGui::IsItemHovered()) ImGui::SetTooltip("Play");

            ImGui::SameLine();

            if (state.simState == SimulationState::Paused)
                ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.9f, 0.8f, 0.2f, 1.0f));

            if (ImGui::Button(SYN_ICON_PAUSE, ImVec2(26, 28))) 
                vm.Dispatch(PauseSimulationIntent{});

            if (state.simState == SimulationState::Paused) 
                ImGui::PopStyleColor();

            if (ImGui::IsItemHovered()) ImGui::SetTooltip("Pause");

            ImGui::SameLine();

            if (ImGui::Button(SYN_ICON_STOP, ImVec2(26, 28))) 
                vm.Dispatch(StopSimulationIntent{});
            if (ImGui::IsItemHovered()) ImGui::SetTooltip("Stop");

            ImGui::PopStyleVar();
            ImGui::PopStyleColor();
        }
        ImGui::EndChild();

        ImGui::PopStyleVar(2);
        ImGui::PopStyleColor();
    }

    void ViewportView::RenderFloatingToolbar(ViewportViewModel& vm, const ViewportState& state, ImVec2 startPos, ImVec2 size) {
        float toolbarWidth = 40.0f;
        float toolbarHeight = 110.0f;

        ImGui::SetCursorScreenPos(ImVec2(startPos.x + size.x - toolbarWidth - 8.0f, startPos.y + 8.0f));

        ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.1f, 0.1f, 0.1f, 0.85f));
        ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, 6.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(4.0f, 6.0f));

        if (ImGui::BeginChild("##FloatingToolbar", ImVec2(toolbarWidth, toolbarHeight), false, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_AlwaysUseWindowPadding)) {

            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 0, 0, 0));
            ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(4.0f, 6.0f));

            if (ImGui::Button(SYN_ICON_ARROWS_ALT, ImVec2(32, 28))) ImGui::OpenPopup("GizmoPopup");
            if (ImGui::IsItemHovered()) ImGui::SetTooltip("Gizmo Settings");

            if (ImGui::Button(SYN_ICON_LAYER_GROUP, ImVec2(32, 28))) ImGui::OpenPopup("ImagePopup");
            if (ImGui::IsItemHovered()) ImGui::SetTooltip("Render Targets & View Modes");

            if (ImGui::Button(SYN_ICON_BUG, ImVec2(32, 28))) ImGui::OpenPopup("DebugPopup");
            if (ImGui::IsItemHovered()) ImGui::SetTooltip("Debug Visibility");

            ImGui::PopStyleVar();
            ImGui::PopStyleColor();

            DrawGizmoPopup(vm, state);
            DrawImagePopup(vm, state);
            DrawDebugPopup(vm, state);
        }
        ImGui::EndChild();

        ImGui::PopStyleVar(2);
        ImGui::PopStyleColor();
    }

    void ViewportView::DrawGizmoPopup(ViewportViewModel& vm, const ViewportState& state) {
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(8.0f, 8.0f));
        if (ImGui::BeginPopup("GizmoPopup")) {
            if (ImGui::BeginChild("##GizmoWindow", ImVec2(240, 210), false)) {

                ImGui::SeparatorText("Operation");

                int mode = static_cast<int>(state.gizmoMode);
                if (ImGui::RadioButton("Local##Gizmo", &mode, ImGuizmo::LOCAL))
                    vm.Dispatch(ChangeGizmoModeIntent{ ImGuizmo::LOCAL });

                ImGui::SameLine();
                if (ImGui::RadioButton("World##Gizmo", &mode, ImGuizmo::WORLD))
                    vm.Dispatch(ChangeGizmoModeIntent{ ImGuizmo::WORLD });

                int op = static_cast<int>(state.gizmoOperation);
                if (ImGui::RadioButton("Translate##Gizmo", &op, ImGuizmo::TRANSLATE))
                    vm.Dispatch(ChangeGizmoOperationIntent{ ImGuizmo::TRANSLATE });
                ImGui::SameLine();
                if (ImGui::RadioButton("Rotate##Gizmo", &op, ImGuizmo::ROTATE))
                    vm.Dispatch(ChangeGizmoOperationIntent{ ImGuizmo::ROTATE });
                ImGui::SameLine();
                if (ImGui::RadioButton("Scale##Gizmo", &op, ImGuizmo::SCALE))
                    vm.Dispatch(ChangeGizmoOperationIntent{ ImGuizmo::SCALE });

                ImGui::SeparatorText("Snapping");

                bool snap = state.useSnap;
                if (ImGui::Checkbox("Enable##Snap", &snap))
                    vm.Dispatch(ToggleSnapIntent{ snap });

                glm::vec3 snapTrans = state.snapTranslate;
                if (ImGui::DragFloat3("Translate##Snap", glm::value_ptr(snapTrans), 0.1f))
                    vm.Dispatch(ChangeSnapTranslateIntent{ snapTrans });

                float snapRot = state.snapAngle;
                if (ImGui::DragFloat("Rotate##Snap", &snapRot, 1.0f))
                    vm.Dispatch(ChangeSnapRotateIntent{ snapRot });

                float snapScl = state.snapScale;
                if (ImGui::DragFloat("Scale##Snap", &snapScl, 0.1f))
                    vm.Dispatch(ChangeSnapScaleIntent{ snapScl });

            }
            ImGui::EndChild();
            ImGui::EndPopup();
        }
        ImGui::PopStyleVar();
    }

    void ViewportView::DrawImagePopup(ViewportViewModel& vm, const ViewportState& state) {
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(8.0f, 8.0f));
        if (ImGui::BeginPopup("ImagePopup")) {
            if (ImGui::BeginChild("##ViewportImage", ImVec2(280, 380), false)) {

                auto RadioButton = [&](const char* label, const std::string& group, const std::string& target, const std::string& view) {
                    bool isActive = (state.currentTarget == target && state.currentView == view);
                    if (ImGui::RadioButton(label, isActive)) {
                        vm.Dispatch(ChangeTargetIntent{ group, target, view });
                    }
                    return isActive;
                    };

                RadioButton("Main", RenderTargetGroupNames::Deferred, RenderTargetNames::Main, Vk::ImageViewNames::Default);

                ImGui::SeparatorText("GBuffer Textures");

                RadioButton("Color", RenderTargetGroupNames::Deferred, RenderTargetNames::ColorMetallic, RenderTargetViewNames::Color);
                RadioButton("Metallic", RenderTargetGroupNames::Deferred, RenderTargetNames::ColorMetallic, RenderTargetViewNames::Metallic);
                RadioButton("Normal", RenderTargetGroupNames::Deferred, RenderTargetNames::NormalRoughness, RenderTargetViewNames::Normal);
                RadioButton("Roughness", RenderTargetGroupNames::Deferred, RenderTargetNames::NormalRoughness, RenderTargetViewNames::Roughness);
                RadioButton("Emissive", RenderTargetGroupNames::Deferred, RenderTargetNames::EmissiveAo, RenderTargetViewNames::Emissive);
                RadioButton("Ambient Occlusion", RenderTargetGroupNames::Deferred, RenderTargetNames::EmissiveAo, RenderTargetViewNames::AmbientOcclusion);
                RadioButton("Ssao", RenderTargetGroupNames::Deferred, RenderTargetNames::SsaoAo, Vk::ImageViewNames::Default);

                ImGui::SeparatorText("Wboit Textures");

                RadioButton("Transparent Accum", RenderTargetGroupNames::Deferred, RenderTargetNames::TransparentAccum, Vk::ImageViewNames::Default);
                RadioButton("Transparent Reveal", RenderTargetGroupNames::Deferred, RenderTargetNames::TransparentReveal, Vk::ImageViewNames::Default);

                ImGui::SeparatorText("Mipchain Textures");

                int maxMipIndex = 0;
                if (state.width > 0 && state.height > 0) {
                    maxMipIndex = static_cast<int>(Vk::ImageUtils::CalculateMipLevels(state.width, state.height)) - 1;
                    maxMipIndex = std::max(0, maxMipIndex);
                }

                static int bloomMip = 0;
                bloomMip = std::min(bloomMip, maxMipIndex);
                std::string bloomView = std::string(Vk::ImageViewNames::Default) + Vk::ImageViewNames::Mip + std::to_string(bloomMip);
                RadioButton("Bloom", RenderTargetGroupNames::Deferred, RenderTargetNames::Bloom, bloomView);

                if (state.currentTarget == RenderTargetNames::Bloom) {
                    ImGui::Indent();
                    if (ImGui::SliderInt("Mip##Bloom", &bloomMip, 0, maxMipIndex)) {
                        bloomView = std::string(Vk::ImageViewNames::Default) + Vk::ImageViewNames::Mip + std::to_string(bloomMip);
                        vm.Dispatch(ChangeTargetIntent{ RenderTargetGroupNames::Deferred, RenderTargetNames::Bloom, bloomView });
                    }
                    ImGui::Unindent();
                }

                static int depthMipMax = 0;
                depthMipMax = std::min(depthMipMax, maxMipIndex);
                std::string depthMaxView = std::string(RenderTargetViewNames::DepthOpaqueMax) + Vk::ImageViewNames::Mip + std::to_string(depthMipMax);
                RadioButton("Depth Pyramid Max", RenderTargetGroupNames::Deferred, RenderTargetNames::DepthPyramid, depthMaxView);

                if (state.currentView.contains(RenderTargetViewNames::DepthOpaqueMax)) {
                    ImGui::Indent();
                    if (ImGui::SliderInt("Mip##DepthMax", &depthMipMax, 0, maxMipIndex)) {
                        depthMaxView = std::string(RenderTargetViewNames::DepthOpaqueMax) + Vk::ImageViewNames::Mip + std::to_string(depthMipMax);
                        vm.Dispatch(ChangeTargetIntent{ RenderTargetGroupNames::Deferred, RenderTargetNames::DepthPyramid, depthMaxView });
                    }
                    ImGui::Unindent();
                }

                static int depthMipMin = 0;
                depthMipMin = std::min(depthMipMin, maxMipIndex);
                std::string depthMinView = std::string(RenderTargetViewNames::DepthTransparentMin) + Vk::ImageViewNames::Mip + std::to_string(depthMipMin);
                RadioButton("Depth Pyramid Min", RenderTargetGroupNames::Deferred, RenderTargetNames::DepthPyramid, depthMinView);

                if (state.currentView.contains(RenderTargetViewNames::DepthTransparentMin)) {
                    ImGui::Indent();
                    if (ImGui::SliderInt("Mip##DepthMin", &depthMipMin, 0, maxMipIndex)) {
                        depthMinView = std::string(RenderTargetViewNames::DepthTransparentMin) + Vk::ImageViewNames::Mip + std::to_string(depthMipMin);
                        vm.Dispatch(ChangeTargetIntent{ RenderTargetGroupNames::Deferred, RenderTargetNames::DepthPyramid, depthMinView });
                    }
                    ImGui::Unindent();
                }

                ImGui::SeparatorText("Shadow Passes");

                static int shadowHzbMaxMip = 0;
                shadowHzbMaxMip = std::clamp(shadowHzbMaxMip, 0, int(SHADOW_HIZ_MIP_LEVELS - 1));
                std::string shadowMaxBaseView = RenderTargetViewNames::DirectionLightShadowDepthPyramidMax;
                std::string shadowMaxView = shadowMaxBaseView + Vk::ImageViewNames::Mip + std::to_string(shadowHzbMaxMip);

                RadioButton("DirLight HZB Max (R)", RenderTargetGroupNames::Deferred, RenderTargetNames::DirectionLightShadowDepthPyramid, shadowMaxView);

                if (state.currentView.contains(shadowMaxBaseView)) {
                    ImGui::Indent();
                    if (ImGui::SliderInt("Mip##ShadowHzbMax", &shadowHzbMaxMip, 0, SHADOW_HIZ_MIP_LEVELS - 1)) {
                        shadowMaxView = shadowMaxBaseView + Vk::ImageViewNames::Mip + std::to_string(shadowHzbMaxMip);
                        vm.Dispatch(ChangeTargetIntent{ RenderTargetGroupNames::Deferred, RenderTargetNames::DirectionLightShadowDepthPyramid, shadowMaxView });
                    }
                    ImGui::Unindent();
                }

                static int shadowHzbMinMip = 0;
                shadowHzbMinMip = std::clamp(shadowHzbMinMip, 0, int(SHADOW_HIZ_MIP_LEVELS - 1));
                std::string shadowMinBaseView = RenderTargetViewNames::DirectionLightShadowDepthPyramidMin;
                std::string shadowMinView = shadowMinBaseView + Vk::ImageViewNames::Mip + std::to_string(shadowHzbMinMip);

                RadioButton("DirLight HZB Min (G)", RenderTargetGroupNames::Deferred, RenderTargetNames::DirectionLightShadowDepthPyramid, shadowMinView);

                if (state.currentView.contains(shadowMinBaseView)) {
                    ImGui::Indent();
                    if (ImGui::SliderInt("Mip##ShadowHzbMin", &shadowHzbMinMip, 0, SHADOW_HIZ_MIP_LEVELS - 1)) {
                        shadowMinView = shadowMinBaseView + Vk::ImageViewNames::Mip + std::to_string(shadowHzbMinMip);
                        vm.Dispatch(ChangeTargetIntent{ RenderTargetGroupNames::Deferred, RenderTargetNames::DirectionLightShadowDepthPyramid, shadowMinView });
                    }
                    ImGui::Unindent();
                }

                static int spotShadowHzbMaxMip = 0;
                spotShadowHzbMaxMip = std::clamp(spotShadowHzbMaxMip, 0, int(SPOT_SHADOW_HIZ_MIP_LEVELS - 1));
                std::string spotShadowMaxBaseView = RenderTargetViewNames::SpotLightShadowDepthPyramidMax;
                std::string spotShadowMaxView = spotShadowMaxBaseView + Vk::ImageViewNames::Mip + std::to_string(spotShadowHzbMaxMip);

                RadioButton("SpotLight HZB Max (R)", RenderTargetGroupNames::Deferred, RenderTargetNames::SpotLightShadowDepthPyramid, spotShadowMaxView);

                if (state.currentView.contains(spotShadowMaxBaseView)) {
                    ImGui::Indent();
                    if (ImGui::SliderInt("Mip##SpotShadowHzbMax", &spotShadowHzbMaxMip, 0, SPOT_SHADOW_HIZ_MIP_LEVELS - 1)) {
                        spotShadowMaxView = spotShadowMaxBaseView + Vk::ImageViewNames::Mip + std::to_string(spotShadowHzbMaxMip);
                        vm.Dispatch(ChangeTargetIntent{ RenderTargetGroupNames::Deferred, RenderTargetNames::SpotLightShadowDepthPyramid, spotShadowMaxView });
                    }
                    ImGui::Unindent();
                }

                static int spotShadowHzbMinMip = 0;
                spotShadowHzbMinMip = std::clamp(spotShadowHzbMinMip, 0, int(SPOT_SHADOW_HIZ_MIP_LEVELS - 1));
                std::string spotShadowMinBaseView = RenderTargetViewNames::SpotLightShadowDepthPyramidMin;
                std::string spotShadowMinView = spotShadowMinBaseView + Vk::ImageViewNames::Mip + std::to_string(spotShadowHzbMinMip);

                RadioButton("SpotLight HZB Min (G)", RenderTargetGroupNames::Deferred, RenderTargetNames::SpotLightShadowDepthPyramid, spotShadowMinView);

                if (state.currentView.contains(spotShadowMinBaseView)) {
                    ImGui::Indent();
                    if (ImGui::SliderInt("Mip##SpotShadowHzbMin", &spotShadowHzbMinMip, 0, SPOT_SHADOW_HIZ_MIP_LEVELS - 1)) {
                        spotShadowMinView = spotShadowMinBaseView + Vk::ImageViewNames::Mip + std::to_string(spotShadowHzbMinMip);
                        vm.Dispatch(ChangeTargetIntent{ RenderTargetGroupNames::Deferred, RenderTargetNames::SpotLightShadowDepthPyramid, spotShadowMinView });
                    }
                    ImGui::Unindent();
                }

                static int pointShadowHzbMaxMip = 0;
                pointShadowHzbMaxMip = std::clamp(pointShadowHzbMaxMip, 0, int(POINT_SHADOW_HIZ_MIP_LEVELS - 1));
                std::string pointShadowMaxBaseView = RenderTargetViewNames::PointLightShadowDepthPyramidMax;
                std::string pointShadowMaxView = pointShadowMaxBaseView + Vk::ImageViewNames::Mip + std::to_string(pointShadowHzbMaxMip);

                RadioButton("PointLight HZB Max (R)", RenderTargetGroupNames::Deferred, RenderTargetNames::PointLightShadowDepthPyramid, pointShadowMaxView);
                if (state.currentView.contains(pointShadowMaxBaseView)) {
                    ImGui::Indent();
                    if (ImGui::SliderInt("Mip##PointShadowHzbMax", &pointShadowHzbMaxMip, 0, POINT_SHADOW_HIZ_MIP_LEVELS - 1)) {
                        pointShadowMaxView = pointShadowMaxBaseView + Vk::ImageViewNames::Mip + std::to_string(pointShadowHzbMaxMip);
                        vm.Dispatch(ChangeTargetIntent{ RenderTargetGroupNames::Deferred, RenderTargetNames::PointLightShadowDepthPyramid, pointShadowMaxView });
                    }
                    ImGui::Unindent();
                }

                static int pointShadowHzbMinMip = 0;
                pointShadowHzbMinMip = std::clamp(pointShadowHzbMinMip, 0, int(POINT_SHADOW_HIZ_MIP_LEVELS - 1));
                std::string pointShadowMinBaseView = RenderTargetViewNames::PointLightShadowDepthPyramidMin;
                std::string pointShadowMinView = pointShadowMinBaseView + Vk::ImageViewNames::Mip + std::to_string(pointShadowHzbMinMip);

                RadioButton("PointLight HZB Min (G)", RenderTargetGroupNames::Deferred, RenderTargetNames::PointLightShadowDepthPyramid, pointShadowMinView);
                if (state.currentView.contains(pointShadowMinBaseView)) {
                    ImGui::Indent();
                    if (ImGui::SliderInt("Mip##PointShadowHzbMin", &pointShadowHzbMinMip, 0, POINT_SHADOW_HIZ_MIP_LEVELS - 1)) {
                        pointShadowMinView = pointShadowMinBaseView + Vk::ImageViewNames::Mip + std::to_string(pointShadowHzbMinMip);
                        vm.Dispatch(ChangeTargetIntent{ RenderTargetGroupNames::Deferred, RenderTargetNames::PointLightShadowDepthPyramid, pointShadowMinView });
                    }
                    ImGui::Unindent();
                }
            }
            ImGui::EndChild();
            ImGui::EndPopup();
        }
        ImGui::PopStyleVar();
    }

    void ViewportView::DrawDebugPopup(ViewportViewModel& vm, const ViewportState& state) {
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(8.0f, 8.0f));
        if (ImGui::BeginPopup("DebugPopup")) {
            if (ImGui::BeginChild("##VisualizationWindow", ImVec2(220, 280), false)) {

                ImGui::SeparatorText("Debug Visibility");

                bool enabled = state.enableDebugVisibility;
                if (ImGui::Checkbox("Enable", &enabled)) {
                    vm.Dispatch(ToggleDebugVisibilityIntent{ enabled });
                }

                ImGui::BeginDisabled(!state.enableDebugVisibility);

                ImGui::SeparatorText("Mode");

                int mode = static_cast<int>(state.debugVisibilityMode);
                auto RadioButton = [&](const char* label, int targetMode) {
                    if (ImGui::RadioButton(label, &mode, targetMode)) {
                        vm.Dispatch(ChangeDebugVisibilityModeIntent{ static_cast<uint32_t>(targetMode) });
                    }
                    };

                RadioButton("Entity ID", 0);
                RadioButton("Pipeline Type", 1);
                RadioButton("LOD Level", 2);
                RadioButton("Mesh Index", 3);
                RadioButton("Meshlet Index", 4);
                RadioButton("Triangle Index", 5);
                RadioButton("All Combined", 6);
                RadioButton("Material Type", 7);
                RadioButton("Tile Slice Count", 8);
                RadioButton("Tile Light Count", 9);

                ImGui::EndDisabled();

            }
            ImGui::EndChild();
            ImGui::EndPopup();
        }
        ImGui::PopStyleVar();
    }

    void ViewportView::DrawGizmo(ViewportViewModel& vm, const ViewportState& state, ImVec2 startPos, ImVec2 size) {
        if (state.activeEntity == NULL_ENTITY)
            return;

        ImGuizmo::SetOrthographic(false);
        ImGuizmo::SetDrawlist();
        ImGuizmo::SetRect(startPos.x, startPos.y, size.x, size.y);

        glm::mat4 cameraView = state.cameraView;
        glm::mat4 cameraProj = state.cameraProj;
        glm::mat4 transform = state.entityWorldTransform;

        float* snapValue = nullptr;
        if (state.useSnap) {
            switch (state.gizmoOperation) {
            case ImGuizmo::TRANSLATE: snapValue = const_cast<float*>(glm::value_ptr(state.snapTranslate)); break;
            case ImGuizmo::ROTATE:    snapValue = const_cast<float*>(&state.snapAngle); break;
            case ImGuizmo::SCALE:     snapValue = const_cast<float*>(&state.snapScale); break;
            }
        }

        ImGuizmo::Manipulate(
            glm::value_ptr(cameraView),
            glm::value_ptr(cameraProj),
            state.gizmoOperation,
            state.gizmoMode,
            glm::value_ptr(transform),
            nullptr, snapValue
        );

        if (ImGuizmo::IsUsing()) {
            vm.Dispatch(ApplyGizmoTransformIntent{ transform });
        }
    }

    void ViewportView::HandleShortcuts(ViewportViewModel& vm) {
        if (ImGui::IsWindowFocused() || ImGui::IsWindowHovered()) {
            if (ImGui::IsKeyPressed(ImGuiKey_1)) vm.Dispatch(ChangeGizmoOperationIntent{ ImGuizmo::TRANSLATE });
            if (ImGui::IsKeyPressed(ImGuiKey_2)) vm.Dispatch(ChangeGizmoOperationIntent{ ImGuizmo::ROTATE });
            if (ImGui::IsKeyPressed(ImGuiKey_3)) vm.Dispatch(ChangeGizmoOperationIntent{ ImGuizmo::SCALE });
        }
    }

}