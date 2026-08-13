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

#include "AnimationSequencerView.h"
#include "Editor/Manager/EditorIcons.h"
#include "Editor/Widgets/CardWidget.h"
#include <imgui.h>
#include <imgui_neo_sequencer.h>

namespace Syn {

    void AnimationSequencerView::Draw(AnimationSequencerViewModel& vm) {
        const auto& state = vm.GetState();

        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(8, 8));
        ImGuiWindowFlags windowFlags = ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse;

        if (ImGui::Begin(SYN_ICON_CLOCK " Sequencer", nullptr, windowFlags)) {

            float mainContentBottomY = ImGui::GetWindowPos().y + ImGui::GetWindowContentRegionMax().y;

            if (Syn::UI::BeginCard("AnimationTimelineCard", SYN_ICON_FILM, _cardState)) {

                if (state.currentAnimData && !state.editableTracks.empty()) {
                    int32_t currentFrame = static_cast<int32_t>(state.currentFrame);
                    int32_t startFrame = 0;
                    int32_t endFrame = static_cast<int32_t>(state.currentAnimData->descriptor.frameCount);

                    float currentY = ImGui::GetCursorScreenPos().y;
                    float sequencerHeight = mainContentBottomY - currentY - 12.0f;
                    if (sequencerHeight < 150.0f) sequencerHeight = 150.0f;

                    ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.08f, 0.08f, 0.08f, 0.6f));
                    ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, 8.0f);
                    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));

                    ImGui::BeginChild("AnimSequencerContainer", ImVec2(0, sequencerHeight), false);

                    ImVec2 seqSize = ImVec2(ImGui::GetContentRegionAvail().x, 0.0f);

                    ImGui::PushNeoSequencerStyleColor(ImGuiNeoSequencerCol_Bg, ImVec4(0.13f, 0.13f, 0.13f, 1.00f));
                    ImGui::PushNeoSequencerStyleColor(ImGuiNeoSequencerCol_TopBarBg, ImVec4(0.11f, 0.11f, 0.11f, 1.00f));
                    ImGui::PushNeoSequencerStyleColor(ImGuiNeoSequencerCol_SelectedTimeline, ImVec4(0.19f, 0.19f, 0.19f, 0.54f)); 
                    ImGui::PushNeoSequencerStyleColor(ImGuiNeoSequencerCol_TimelineBorder, ImVec4(0.08f, 0.08f, 0.08f, 1.00f)); 
                    ImGui::PushNeoSequencerStyleColor(ImGuiNeoSequencerCol_TimelinesBg, ImVec4(0.11f, 0.11f, 0.11f, 1.00f)); 

                    ImGui::PushNeoSequencerStyleColor(ImGuiNeoSequencerCol_FramePointer, ImVec4(0.26f, 0.59f, 0.98f, 1.00f));
                    ImGui::PushNeoSequencerStyleColor(ImGuiNeoSequencerCol_FramePointerHovered, ImVec4(0.36f, 0.69f, 1.00f, 1.00f));
                    ImGui::PushNeoSequencerStyleColor(ImGuiNeoSequencerCol_FramePointerPressed, ImVec4(0.16f, 0.49f, 0.88f, 1.00f));

                    ImGui::PushNeoSequencerStyleColor(ImGuiNeoSequencerCol_Keyframe, ImVec4(0.50f, 0.50f, 0.50f, 1.00f));
                    ImGui::PushNeoSequencerStyleColor(ImGuiNeoSequencerCol_KeyframeHovered, ImVec4(0.95f, 0.96f, 0.98f, 1.00f));
                    ImGui::PushNeoSequencerStyleColor(ImGuiNeoSequencerCol_KeyframePressed, ImVec4(0.26f, 0.59f, 0.98f, 1.00f)); 
                    ImGui::PushNeoSequencerStyleColor(ImGuiNeoSequencerCol_KeyframeSelected, ImVec4(0.26f, 0.59f, 0.98f, 1.00f));

                    ImGui::PushNeoSequencerStyleColor(ImGuiNeoSequencerCol_ZoomBarBg, ImVec4(0.05f, 0.05f, 0.05f, 0.54f));
                    ImGui::PushNeoSequencerStyleColor(ImGuiNeoSequencerCol_ZoomBarSlider, ImVec4(0.20f, 0.20f, 0.20f, 1.00f));
                    ImGui::PushNeoSequencerStyleColor(ImGuiNeoSequencerCol_ZoomBarSliderHovered, ImVec4(0.28f, 0.28f, 0.28f, 1.00f)); 

                    if (ImGui::BeginNeoSequencer("AnimationSequencer", &currentFrame, &startFrame, &endFrame, seqSize)) {

                        for (size_t i = 0; i < state.editableTracks.size(); ++i) {
                            const auto& track = state.editableTracks[i];
                            const auto& trackUI = state.trackUIStates[i];

                            ImGui::PushID(static_cast<int>(i));

                            bool groupOpen = trackUI.groupOpen;
                            if (ImGui::BeginNeoGroup(track.nodeName.c_str(), &groupOpen)) {

                                std::vector<int32_t> posFrames = trackUI.positionFrames;
                                std::vector<int32_t> rotFrames = trackUI.rotationFrames;
                                std::vector<int32_t> scaleFrames = trackUI.scaleFrames;

                                if (ImGui::BeginNeoTimeline(("Position##" + std::to_string(i)).c_str(), posFrames, nullptr)) {
                                    ImGui::EndNeoTimeLine();
                                }

                                if (ImGui::BeginNeoTimeline(("Rotation##" + std::to_string(i)).c_str(), rotFrames, nullptr)) {
                                    ImGui::EndNeoTimeLine();
                                }

                                if (ImGui::BeginNeoTimeline(("Scale##" + std::to_string(i)).c_str(), scaleFrames, nullptr)) {
                                    ImGui::EndNeoTimeLine();
                                }

                                ImGui::EndNeoGroup();

                                if (posFrames != trackUI.positionFrames ||
                                    rotFrames != trackUI.rotationFrames ||
                                    scaleFrames != trackUI.scaleFrames) {
                                    vm.Dispatch(UpdateTrackKeysIntent{ static_cast<uint32_t>(i), posFrames, rotFrames, scaleFrames });
                                }
                            }

                            if (groupOpen != trackUI.groupOpen) {
                                vm.Dispatch(ToggleSequencerGroupIntent{ static_cast<uint32_t>(i), groupOpen });
                            }

                            ImGui::PopID();
                        }
                        ImGui::EndNeoSequencer();
                    }

                    ImGui::PopNeoSequencerStyleColor(15);

                    ImGui::EndChild();

                    ImGui::PopStyleVar(2);
                    ImGui::PopStyleColor();

                    if (currentFrame != state.currentFrame) {
                        vm.Dispatch(ChangeSequencerFrameIntent{ currentFrame });
                    }
                }
                else {
                    ImGui::Spacing();
                    ImGui::Indent(8.0f);
                    ImGui::TextDisabled("Select an animation to edit tracks.");
                    ImGui::Unindent(8.0f);
                }
            }
            Syn::UI::EndCard();
        }
        ImGui::End();
        ImGui::PopStyleVar();
    }
}