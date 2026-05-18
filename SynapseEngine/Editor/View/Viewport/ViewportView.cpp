#include "ViewportView.h"
#include "Engine/Vk/Image/ImageUtils.h"
#include <ImGuizmo.h>
#include <glm/gtc/type_ptr.hpp>

namespace Syn {

    void ViewportView::Draw(ViewportViewModel& vm) {
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2{ 0, 0 });

        ImGui::Begin("Viewport", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_MenuBar);

        ViewportState state = vm.GetState();

        if (ImGui::BeginMenuBar()) {
            DrawGizmoMenu(vm, state);
            DrawImageMenu(vm, state);
            DrawDebugMenu(vm, state);
            ImGui::EndMenuBar();
        }

        ImVec2 viewportPanelSize = ImGui::GetContentRegionAvail();
        uint32_t currentWidth = static_cast<uint32_t>(viewportPanelSize.x);
        uint32_t currentHeight = static_cast<uint32_t>(viewportPanelSize.y);

        bool isResizing = (currentWidth > 0 && currentHeight > 0 && (currentWidth != state.width || currentHeight != state.height));

        vm.Dispatch(ResizeViewportIntent{ currentWidth, currentHeight });

        ImVec2 imageStartPos = ImGui::GetCursorScreenPos();
        if (state.textureId && !isResizing) {
            ImGui::Image(state.textureId, viewportPanelSize);
        }
        else {
            ImGui::Dummy(viewportPanelSize);
        }

        ImVec2 vMin = ImGui::GetItemRectMin();
        ImVec2 vMax = ImGui::GetItemRectMax();

        if (ImGui::IsItemHovered() && ImGui::IsMouseClicked(ImGuiMouseButton_Left) && !ImGuizmo::IsOver()) {
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

    void ViewportView::DrawGizmoMenu(ViewportViewModel& vm, const ViewportState& state) {
        if (ImGui::BeginMenu("Gizmo")) {
            ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(5.0f, 0.0f));
            if (ImGui::BeginChild("##GizmoWindow", ImVec2(255, 210), ImGuiChildFlags_AlwaysUseWindowPadding)) {

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
                if (ImGui::DragFloat3("Translate##Snap", glm::value_ptr(snapTrans), 0.1f)) {
                    vm.Dispatch(ChangeSnapTranslateIntent{ snapTrans });
                }

                float snapRot = state.snapAngle;
                if (ImGui::DragFloat("Rotate##Snap", &snapRot, 1.0f)) {
                    vm.Dispatch(ChangeSnapRotateIntent{ snapRot });
                }

                float snapScl = state.snapScale;
                if (ImGui::DragFloat("Scale##Snap", &snapScl, 0.1f)) {
                    vm.Dispatch(ChangeSnapScaleIntent{ snapScl });
                }

                ImGui::EndChild();
            }
            ImGui::PopStyleVar();
            ImGui::EndMenu();
        }
    }

    void ViewportView::DrawImageMenu(ViewportViewModel& vm, const ViewportState& state) {
        if (ImGui::BeginMenu("Image")) {
            ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(5.0f, 0.0f));

            if (ImGui::BeginChild("##ViewportImage", ImVec2(265, 380), ImGuiChildFlags_AlwaysUseWindowPadding)) {

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
                RadioButton("DP-HVO", RenderTargetGroupNames::Deferred, RenderTargetNames::VolumetricAo, Vk::ImageViewNames::Default);

                ImGui::SeparatorText("Wboit Textures");

                RadioButton("Transparent Accum", RenderTargetGroupNames::Deferred, RenderTargetNames::TransparentAccum, Vk::ImageViewNames::Default);
                RadioButton("Transparent Reveal", RenderTargetGroupNames::Deferred, RenderTargetNames::TransparentReveal, Vk::ImageViewNames::Default);

                ImGui::SeparatorText("Mipchain Textures");

                ImGui::Separator();

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

                ImGui::EndChild();
            }
            ImGui::PopStyleVar();
            ImGui::EndMenu();
        }
    }

    void ViewportView::DrawDebugMenu(ViewportViewModel& vm, const ViewportState& state) {
        if (ImGui::BeginMenu("Debug")) {
            ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(5.0f, 0.0f));

            if (ImGui::BeginChild("##VisualizationWindow", ImVec2(240, 260), ImGuiChildFlags_AlwaysUseWindowPadding)) {

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

                ImGui::EndChild();
            }
            ImGui::PopStyleVar();
            ImGui::EndMenu();
        }
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