#include "PipelineOverrideView.h"
#include "Editor/Manager/EditorIcons.h"
#include "Editor/Widgets/CardWidget.h"
#include "Editor/Widgets/PropertyGrid.h"
#include <imgui.h>
#include <string>

namespace Syn {
    void PipelineOverrideView::Draw(PipelineOverrideViewModel& vm) {
        PipelineOverrideState state = vm.GetState();
        if (!state.hasComponent) return;

        constexpr const char* CardTitle = "Pipeline Override";
        if (Syn::UI::BeginCard(CardTitle, SYN_ICON_MAGIC, _isCardOpen)) {
            if (Syn::UI::BeginPropertyGrid("PipelineOverrideGrid"))
            {
                std::string sharedPreview = "None";
                if (state.sharedPipelineEntity != NULL_ENTITY) {
                    for (const auto& ent : state.compatibleSharedEntities) {
                        if (ent.first == state.sharedPipelineEntity) {
                            sharedPreview = ent.second;
                            break;
                        }
                    }
                }

                if (Syn::UI::BeginPropertyCombo("Shared Entity", sharedPreview.c_str()))
                {
                    if (ImGui::Selectable("None", state.sharedPipelineEntity == NULL_ENTITY)) {
                        vm.Dispatch(SetSharedPipelineEntityIntent{ NULL_ENTITY });
                    }

                    ImGuiListClipper clipper;
                    clipper.Begin(static_cast<int>(state.compatibleSharedEntities.size()));

                    while (clipper.Step()) {
                        for (int i = clipper.DisplayStart; i < clipper.DisplayEnd; ++i) {
                            const auto& ent = state.compatibleSharedEntities[i];
                            bool isSelected = (state.sharedPipelineEntity == ent.first);

                            ImGui::PushID(ent.first);
                            if (ImGui::Selectable(ent.second.c_str(), isSelected)) {
                                vm.Dispatch(SetSharedPipelineEntityIntent{ ent.first });
                            }
                            if (isSelected) {
                                ImGui::SetItemDefaultFocus();
                            }
                            ImGui::PopID();
                        }
                    }

                    Syn::UI::EndPropertyCombo();
                }

                Syn::UI::PropertySeparator();

                if (state.expectedSlotCount == 0) {
                    Syn::UI::PropertyText("Info", "No valid ModelComponent found or Model has 0 meshes.");
                }
                else {
                    for (uint32_t i = 0; i < state.expectedSlotCount; ++i) {
                        uint32_t currentPipeId = state.overrides[i];
                        std::string label = "Slot " + std::to_string(i);

                        std::string previewName = "-";
                        if (currentPipeId != UINT32_MAX) {
                            for (const auto& pipe : state.availablePipelines) {
                                if (pipe.first == currentPipeId) {
                                    previewName = pipe.second;
                                    break;
                                }
                            }
                        }

                        if (Syn::UI::BeginPropertyCombo(label.c_str(), previewName.c_str())) {
                            bool isNoneSelected = (currentPipeId == UINT32_MAX);

                            if (ImGui::Selectable("-", isNoneSelected)) {
                                vm.Dispatch(SetPipelineOverrideSlotIntent{ i, UINT32_MAX });
                            }

                            for (const auto& pipe : state.availablePipelines) {
                                bool isSelected = (currentPipeId == pipe.first);

                                if (ImGui::Selectable(pipe.second.c_str(), isSelected)) {
                                    vm.Dispatch(SetPipelineOverrideSlotIntent{ i, pipe.first });
                                }

                                if (isSelected) {
                                    ImGui::SetItemDefaultFocus();
                                }
                            }
                            Syn::UI::EndPropertyCombo();
                        }
                    }
                }

                Syn::UI::EndPropertyGrid();
            }
        }
        Syn::UI::EndCard();
    }
}