#pragma once
#include "../IView.h"
#include "EditorCore/ViewModels/Viewport/ViewportViewModel.h"
#include <imgui.h>
#include <ImGuizmo.h>
#include <glm/gtc/type_ptr.hpp>

namespace Syn {
    class ViewportView : public IView<ViewportViewModel> {
    public:
        void Draw(ViewportViewModel& vm) override {
            ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2{ 0, 0 });

            ImGui::Begin("Viewport", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_MenuBar);

            ViewportState state = vm.GetState();

            if (ImGui::BeginMenuBar()) {
                DrawGizmoMenu(vm, state);
                DrawImageMenu(vm, state);
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

    private:
        void DrawGizmoMenu(ViewportViewModel& vm, const ViewportState& state) {
            if (ImGui::BeginMenu("Gizmo")) {
                ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(5.0f, 0.0f));
                if (ImGui::BeginChild("##GizmoWindow", ImVec2(255, 210), ImGuiChildFlags_AlwaysUseWindowPadding)) {

                    ImGui::SeparatorText("Operation");

                    int mode = static_cast<int>(state.gizmoMode);
                    if (ImGui::RadioButton("Local", &mode, ImGuizmo::LOCAL)) 
                        vm.Dispatch(ChangeGizmoModeIntent{ ImGuizmo::LOCAL });
                    
                    ImGui::SameLine();

                    if (ImGui::RadioButton("World", &mode, ImGuizmo::WORLD)) 
                        vm.Dispatch(ChangeGizmoModeIntent{ ImGuizmo::WORLD });

                    int op = static_cast<int>(state.gizmoOperation);

                    if (ImGui::RadioButton("Translate", &op, ImGuizmo::TRANSLATE)) 
                        vm.Dispatch(ChangeGizmoOperationIntent{ ImGuizmo::TRANSLATE });

                    ImGui::SameLine();

                    if (ImGui::RadioButton("Rotate", &op, ImGuizmo::ROTATE))
                        vm.Dispatch(ChangeGizmoOperationIntent{ ImGuizmo::ROTATE });

                    ImGui::SameLine();

                    if (ImGui::RadioButton("Scale", &op, ImGuizmo::SCALE)) 
                        vm.Dispatch(ChangeGizmoOperationIntent{ ImGuizmo::SCALE });

                    ImGui::SeparatorText("Snapping");

                    bool snap = state.useSnap;
                    ImGui::Text("Enable"); 
                    ImGui::SameLine(80);
                    if (ImGui::Checkbox("##EnableSnap", &snap)) 
                        vm.Dispatch(ToggleSnapIntent{ snap });

                    ViewportState& mutableState = const_cast<ViewportState&>(state);
                    ImGui::Text("Translate"); 
                    ImGui::SameLine(80); 
                    ImGui::DragFloat3("##TranslateSnap", mutableState.snapTranslate);

                    ImGui::Text("Rotate");    
                    ImGui::SameLine(80);
                    ImGui::DragFloat("##RotateSnap", &mutableState.snapAngle);

                    ImGui::Text("Scale");     
                    ImGui::SameLine(80);
                    ImGui::DragFloat("##ScaleSnap", &mutableState.snapScale);

                    ImGui::EndChild();
                }
                ImGui::PopStyleVar();
                ImGui::EndMenu();
            }
        }

        void DrawImageMenu(ViewportViewModel& vm, const ViewportState& state) {
            if (ImGui::BeginMenu("Image")) {
                ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(5.0f, 0.0f));
                if (ImGui::BeginChild("##ViewportImage", ImVec2(255, 210), ImGuiChildFlags_AlwaysUseWindowPadding)) {

                    auto RadioButton = [&](const char* label, const std::string& group, const std::string& target, const std::string& view) {
                        bool isActive = (state.currentTarget == target && state.currentView == view);
                        if (ImGui::RadioButton(label, isActive)) {
                            vm.Dispatch(ChangeTargetIntent{ group, target, view });
                        }
                        };

                    RadioButton("Main", RenderTargetGroupNames::Deferred, RenderTargetNames::Main, Vk::ImageViewNames::Default);
                    RadioButton("Depth", RenderTargetGroupNames::Deferred, RenderTargetNames::Depth, Vk::ImageViewNames::Default);

                    ImGui::Separator();

                    RadioButton("Color", RenderTargetGroupNames::Deferred, RenderTargetNames::ColorMetallic, RenderTargetViewNames::Color);
                    RadioButton("Metallic", RenderTargetGroupNames::Deferred, RenderTargetNames::ColorMetallic, RenderTargetViewNames::Metallic);
                    RadioButton("Normal", RenderTargetGroupNames::Deferred, RenderTargetNames::NormalRoughness, RenderTargetViewNames::Normal);
                    RadioButton("Roughness", RenderTargetGroupNames::Deferred, RenderTargetNames::NormalRoughness, RenderTargetViewNames::Roughness);
                    RadioButton("Emissive", RenderTargetGroupNames::Deferred, RenderTargetNames::EmissiveAo, RenderTargetViewNames::Emissive);
                    RadioButton("Ambient Occlusion", RenderTargetGroupNames::Deferred, RenderTargetNames::EmissiveAo, RenderTargetViewNames::AmbientOcclusion);

                    ImGui::EndChild();
                }
                ImGui::PopStyleVar();
                ImGui::EndMenu();
            }
        }

        void DrawGizmo(ViewportViewModel& vm, const ViewportState& state, ImVec2 startPos, ImVec2 size) {
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
                case ImGuizmo::TRANSLATE: snapValue = const_cast<float*>(state.snapTranslate); break;
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

        void HandleShortcuts(ViewportViewModel& vm) {
            if (ImGui::IsWindowFocused() || ImGui::IsWindowHovered()) {
                if (ImGui::IsKeyPressed(ImGuiKey_1)) vm.Dispatch(ChangeGizmoOperationIntent{ ImGuizmo::TRANSLATE });
                if (ImGui::IsKeyPressed(ImGuiKey_2)) vm.Dispatch(ChangeGizmoOperationIntent{ ImGuizmo::ROTATE });
                if (ImGui::IsKeyPressed(ImGuiKey_3)) vm.Dispatch(ChangeGizmoOperationIntent{ ImGuizmo::SCALE });
            }
        }
    };
}