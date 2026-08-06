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

                if (state.currentAnimData) {
                    int32_t currentFrame = static_cast<int32_t>(state.currentFrame);
                    int32_t startFrame = 0;
                    int32_t endFrame = static_cast<int32_t>(state.currentAnimData->descriptor.frameCount);

                    float currentY = ImGui::GetCursorScreenPos().y;
                    float sequencerHeight = mainContentBottomY - currentY - 12.0f;
                    if (sequencerHeight < 150.0f) sequencerHeight = 150.0f;

                    ImGui::BeginChild("AnimSequencerContainer", ImVec2(0, sequencerHeight), false, ImGuiWindowFlags_NoScrollbar);

                    if (ImGui::BeginNeoSequencer("AnimationSequencer", &currentFrame, &startFrame, &endFrame, ImGui::GetContentRegionAvail())) {

                        float sampleRate = state.currentAnimData->descriptor.sampleRate;

                        for (size_t i = 0; i < state.currentAnimData->tracks.size(); ++i) {
                            const auto& track = state.currentAnimData->tracks[i];

                            bool groupOpen = true;
                            if (ImGui::BeginNeoGroup(track.nodeName.c_str(), &groupOpen)) {

                                {
                                    std::vector<int32_t> keyframes;
                                    keyframes.reserve(track.positions.size());
                                    for (const auto& key : track.positions) {
                                        keyframes.push_back(static_cast<int32_t>(key.time * sampleRate));
                                    }

                                    if (ImGui::BeginNeoTimeline("Position", keyframes, nullptr)) {
                                        ImGui::EndNeoTimeLine();
                                    }
                                }

                                {
                                    std::vector<int32_t> keyframes;
                                    keyframes.reserve(track.rotations.size());
                                    for (const auto& key : track.rotations) {
                                        keyframes.push_back(static_cast<int32_t>(key.time * sampleRate));
                                    }

                                    if (ImGui::BeginNeoTimeline("Rotation", keyframes, nullptr)) {
                                        ImGui::EndNeoTimeLine();
                                    }
                                }

                                {
                                    std::vector<int32_t> keyframes;
                                    keyframes.reserve(track.scales.size());
                                    for (const auto& key : track.scales) {
                                        keyframes.push_back(static_cast<int32_t>(key.time * sampleRate));
                                    }

                                    if (ImGui::BeginNeoTimeline("Scale", keyframes, nullptr)) {
                                        ImGui::EndNeoTimeLine();
                                    }
                                }

                                ImGui::EndNeoGroup();
                            }
                        }
                        ImGui::EndNeoSequencer();
                    }

                    ImGui::EndChild();

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