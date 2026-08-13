// Copyright (C) 2026 Tamás Péter
// This file is part of SynapseEngine.
//
// SynapseEngine is free software: you can redistribute it and/or modify
// it under the terms of the GNU Affero General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// SynapseEngine is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License
// along with SynapseEngine. If not, see <https://www.gnu.org/licenses/>.

#include "AnimationViewportView.h"
#include "Engine/Vk/Image/ImageUtils.h"
#include "Editor/Manager/EditorIcons.h"
#include <ImGuizmo.h>
#include <imgui.h>
#include <imgui_internal.h>
#include <glm/gtc/type_ptr.hpp>
#include "Engine/Scene/DrawData/SceneDrawData.h"

namespace Syn {

    void AnimationViewportView::Draw(AnimationViewportViewModel& vm) {
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2{ 0, 0 });

        ImGui::Begin(SYN_ICON_GAMEPAD " Animation Viewport", nullptr);

        AnimationViewportState state = vm.GetState();

        ImVec2 viewportPanelSize = ImGui::GetContentRegionAvail();
        uint32_t currentWidth = static_cast<uint32_t>(viewportPanelSize.x);
        uint32_t currentHeight = static_cast<uint32_t>(viewportPanelSize.y);

        bool isResizing = (currentWidth > 0 && currentHeight > 0 && (currentWidth != state.width || currentHeight != state.height));

        vm.Dispatch(ResizeAnimationViewportIntent{ currentWidth, currentHeight });

        ImVec2 imageStartPos = ImGui::GetCursorScreenPos();

        if (viewportPanelSize.x <= 0.0f) viewportPanelSize.x = 1.0f;
        if (viewportPanelSize.y <= 0.0f) viewportPanelSize.y = 1.0f;

        if (state.textureId && !isResizing) {
            ImGui::GetWindowDrawList()->AddCallback(ImGui::GetPlatformIO().DrawCallback_SetSamplerNearest, nullptr);
            ImGui::Image(state.textureId, viewportPanelSize);
            ImGui::GetWindowDrawList()->AddCallback(ImGui::GetPlatformIO().DrawCallback_SetSamplerLinear, nullptr);
        }
        else {
            ImGui::Dummy(viewportPanelSize);
        }

        bool isImageHovered = ImGui::IsItemHovered();

        RenderFloatingToolbar(vm, state, imageStartPos, viewportPanelSize);

        DrawGizmo(vm, state, imageStartPos, viewportPanelSize);

        if (!ImGuizmo::IsUsing()) {
            if (isImageHovered && ImGui::IsMouseClicked(ImGuiMouseButton_Left) && !ImGuizmo::IsOver() && !ImGui::IsAnyItemHovered()) {
                ImVec2 mousePos = ImGui::GetMousePos();
                uint32_t x = static_cast<uint32_t>(mousePos.x - imageStartPos.x);
                uint32_t y = static_cast<uint32_t>(mousePos.y - imageStartPos.y);
                vm.Dispatch(PickAnimationMeshIntent{ x, y });
            }
        }

        HandleShortcuts(vm);

        ImGui::End();
        ImGui::PopStyleVar();
    }

    void AnimationViewportView::RenderFloatingToolbar(AnimationViewportViewModel& vm, const AnimationViewportState& state, ImVec2 startPos, ImVec2 size) {
        float toolbarWidth = 40.0f;
        float toolbarHeight = 110.0f;

        ImGui::SetCursorScreenPos(ImVec2(startPos.x + size.x - toolbarWidth - 8.0f, startPos.y + 8.0f));

        ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.1f, 0.1f, 0.1f, 0.85f));
        ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, 6.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(4.0f, 6.0f));

        if (ImGui::BeginChild("##AnimationFloatingToolbar", ImVec2(toolbarWidth, toolbarHeight), ImGuiChildFlags_AlwaysUseWindowPadding, ImGuiWindowFlags_NoScrollbar)) {

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

    void AnimationViewportView::DrawGizmoPopup(AnimationViewportViewModel& vm, const AnimationViewportState& state) {
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(8.0f, 8.0f));
        if (ImGui::BeginPopup("GizmoPopup")) {
            if (ImGui::BeginChild("##GizmoWindow", ImVec2(240, 210), false)) {

                ImGui::SeparatorText("Operation");

                int mode = static_cast<int>(state.gizmoMode);
                if (ImGui::RadioButton("Local##Gizmo", &mode, ImGuizmo::LOCAL))
                    vm.Dispatch(ChangeAnimationGizmoModeIntent{ ImGuizmo::LOCAL });

                ImGui::SameLine();
                if (ImGui::RadioButton("World##Gizmo", &mode, ImGuizmo::WORLD))
                    vm.Dispatch(ChangeAnimationGizmoModeIntent{ ImGuizmo::WORLD });

                int op = static_cast<int>(state.gizmoOperation);
                if (ImGui::RadioButton("Translate##Gizmo", &op, ImGuizmo::TRANSLATE))
                    vm.Dispatch(ChangeAnimationGizmoOperationIntent{ ImGuizmo::TRANSLATE });
                ImGui::SameLine();
                if (ImGui::RadioButton("Rotate##Gizmo", &op, ImGuizmo::ROTATE))
                    vm.Dispatch(ChangeAnimationGizmoOperationIntent{ ImGuizmo::ROTATE });
                ImGui::SameLine();
                if (ImGui::RadioButton("Scale##Gizmo", &op, ImGuizmo::SCALE))
                    vm.Dispatch(ChangeAnimationGizmoOperationIntent{ ImGuizmo::SCALE });

                ImGui::SeparatorText("Snapping");

                bool snap = state.useSnap;
                if (ImGui::Checkbox("Enable##Snap", &snap))
                    vm.Dispatch(ToggleAnimationSnapIntent{ snap });

                glm::vec3 snapTrans = state.snapTranslate;
                if (ImGui::DragFloat3("Translate##Snap", glm::value_ptr(snapTrans), 0.1f))
                    vm.Dispatch(ChangeAnimationSnapTranslateIntent{ snapTrans });

                float snapRot = state.snapAngle;
                if (ImGui::DragFloat("Rotate##Snap", &snapRot, 1.0f))
                    vm.Dispatch(ChangeAnimationSnapRotateIntent{ snapRot });

                float snapScl = state.snapScale;
                if (ImGui::DragFloat("Scale##Snap", &snapScl, 0.1f))
                    vm.Dispatch(ChangeAnimationSnapScaleIntent{ snapScl });

            }
            ImGui::EndChild();
            ImGui::EndPopup();
        }
        ImGui::PopStyleVar();
    }

    void AnimationViewportView::DrawImagePopup(AnimationViewportViewModel& vm, const AnimationViewportState& state) {
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(8.0f, 8.0f));
        if (ImGui::BeginPopup("ImagePopup")) {
            if (ImGui::BeginChild("##ViewportImage", ImVec2(280, 200), false)) {

                auto RadioButton = [&](const char* label, const std::string& group, const std::string& target, const std::string& view) {
                    bool isActive = (state.currentTarget == target && state.currentView == view);
                    if (ImGui::RadioButton(label, isActive)) {
                        vm.Dispatch(ChangeAnimationTargetIntent{ group, target, view });
                    }
                    return isActive;
                    };

                RadioButton("Main", RenderTargetGroupNames::Main, RenderTargetNames::Main, Vk::ImageViewNames::Default);

                ImGui::SeparatorText("GBuffer Textures");

                RadioButton("Color", RenderTargetGroupNames::Main, RenderTargetNames::ColorMetallic, RenderTargetViewNames::Color);
                RadioButton("Metallic", RenderTargetGroupNames::Main, RenderTargetNames::ColorMetallic, RenderTargetViewNames::Metallic);
                RadioButton("Normal", RenderTargetGroupNames::Main, RenderTargetNames::NormalRoughness, RenderTargetViewNames::Normal);
                RadioButton("Roughness", RenderTargetGroupNames::Main, RenderTargetNames::NormalRoughness, RenderTargetViewNames::Roughness);
            }
            ImGui::EndChild();
            ImGui::EndPopup();
        }
        ImGui::PopStyleVar();
    }

    void AnimationViewportView::DrawDebugPopup(AnimationViewportViewModel& vm, const AnimationViewportState& state) {
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(8.0f, 8.0f));
        if (ImGui::BeginPopup("DebugPopup")) {
            if (ImGui::BeginChild("##VisualizationWindow", ImVec2(220, 280), false)) {

                ImGui::SeparatorText("Debug Visibility");

                bool enabled = state.enableDebugVisibility;
                if (ImGui::Checkbox("Enable", &enabled)) {
                    vm.Dispatch(ToggleAnimationDebugVisibilityIntent{ enabled });
                }

                ImGui::BeginDisabled(!state.enableDebugVisibility);

                ImGui::SeparatorText("Mode");

                int mode = static_cast<int>(state.debugVisibilityMode);
                auto RadioButton = [&](const char* label, int targetMode) {
                    if (ImGui::RadioButton(label, &mode, targetMode)) {
                        vm.Dispatch(ChangeAnimationDebugVisibilityModeIntent{ static_cast<uint32_t>(targetMode) });
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

                ImGui::EndDisabled();

            }
            ImGui::EndChild();
            ImGui::EndPopup();
        }
        ImGui::PopStyleVar();
    }

    void AnimationViewportView::DrawGizmo(AnimationViewportViewModel& vm, const AnimationViewportState& state, ImVec2 startPos, ImVec2 size) {
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
            vm.Dispatch(ApplyAnimationGizmoTransformIntent{ transform });
        }
    }

    void AnimationViewportView::HandleShortcuts(AnimationViewportViewModel& vm) {
        if (ImGui::IsWindowFocused() || ImGui::IsWindowHovered()) {
            if (ImGui::IsKeyPressed(ImGuiKey_1)) vm.Dispatch(ChangeAnimationGizmoOperationIntent{ ImGuizmo::TRANSLATE });
            if (ImGui::IsKeyPressed(ImGuiKey_2)) vm.Dispatch(ChangeAnimationGizmoOperationIntent{ ImGuizmo::ROTATE });
            if (ImGui::IsKeyPressed(ImGuiKey_3)) vm.Dispatch(ChangeAnimationGizmoOperationIntent{ ImGuizmo::SCALE });
        }
    }

}