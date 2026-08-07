#include "AudioViewportView.h"
#include "Editor/Manager/EditorIcons.h"
#include "Editor/Widgets/CardWidget.h"
#include <imgui.h>
#include <imgui_neo_sequencer.h>

namespace Syn {

    void AudioViewportView::Draw(AudioViewportViewModel& vm) {
        const auto& state = vm.GetState();

        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(8, 8));
        ImGuiWindowFlags windowFlags = ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse;

        if (ImGui::Begin("Audio Player", nullptr, windowFlags)) {

            float mainContentBottomY = ImGui::GetWindowPos().y + ImGui::GetWindowContentRegionMax().y;

            if (Syn::UI::BeginCard("AudioPlayerCard", SYN_ICON_PLAY, _cardState)) {

                if (state.currentAudioData && state.duration > 0.0f) {

                    float availWidth = ImGui::GetContentRegionAvail().x;

                    ImGui::AlignTextToFramePadding();
                    ImGui::TextDisabled("Time: %.2fs / %.2fs  |  Sample Rate: %d Hz  |  Channels: %d",
                        state.currentTime, state.duration, state.currentAudioData->sampleRate, state.currentAudioData->channels);

                    ImGui::SameLine();

                    float btnSize = ImGui::GetFrameHeight();
                    float spacing = ImGui::GetStyle().ItemSpacing.x;
                    float totalBtnWidth = (btnSize * 2.0f) + spacing;
                    float centerX = (availWidth - totalBtnWidth) * 0.5f;
                    float textEndX = ImGui::GetCursorPosX();
                    if (centerX < textEndX) centerX = textEndX + spacing;

                    ImGui::SetCursorPosX(centerX);

                    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 0, 0, 0));
                    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(4, 4));

                    if (state.isPlaying) {
                        if (ImGui::Button(SYN_ICON_PAUSE)) vm.Dispatch(AudioViewportPauseIntent{});
                    }
                    else {
                        if (ImGui::Button(SYN_ICON_PLAY)) vm.Dispatch(AudioViewportPlayIntent{});
                    }

                    ImGui::SameLine();
                    if (ImGui::Button(SYN_ICON_STOP)) vm.Dispatch(AudioViewportStopIntent{});

                    ImGui::PopStyleVar();
                    ImGui::PopStyleColor();
                    ImGui::Spacing();

                    float currentY = ImGui::GetCursorScreenPos().y;
                    float availableHeight = mainContentBottomY - currentY - 12.0f;
                    if (availableHeight < 150.0f) availableHeight = 150.0f;

                    float sequencerHeight = 75.0f;
                    float itemSpacingY = ImGui::GetStyle().ItemSpacing.y;

                    float separatorSpace = 12.0f;
                    float waveformHeight = availableHeight - sequencerHeight - separatorSpace - itemSpacingY;
                    if (waveformHeight < 50.0f) waveformHeight = 50.0f;

                    const auto& waveform = state.currentAudioData->waveform;

                    ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.08f, 0.08f, 0.08f, 0.6f));
                    ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, 4.0f);
                    ImGui::BeginChild("WaveformDisplay", ImVec2(0, waveformHeight), false, ImGuiWindowFlags_NoScrollbar);

                    ImVec2 cursorScreenPos = ImGui::GetCursorScreenPos();
                    ImVec2 availSize = ImGui::GetContentRegionAvail();
                    ImDrawList* drawList = ImGui::GetWindowDrawList();

                    if (!waveform.empty()) {
                        float midY = cursorScreenPos.y + (availSize.y * 0.5f);
                        float stepX = availSize.x / static_cast<float>(waveform.size());

                        ImU32 waveColor = IM_COL32(66, 150, 250, 255);
                        ImU32 playedColor = IM_COL32(180, 200, 255, 255);

                        float playProgress = state.currentTime / state.duration;
                        uint32_t playIndex = static_cast<uint32_t>(playProgress * waveform.size());

                        for (size_t i = 0; i < waveform.size(); ++i) {
                            float x = cursorScreenPos.x + (i * stepX);

                            float yMin = midY - (waveform[i].maxAmp * (availSize.y * 0.5f));
                            float yMax = midY - (waveform[i].minAmp * (availSize.y * 0.5f));

                            if (yMin == yMax) {
                                yMin -= 0.5f; yMax += 0.5f;
                            }

                            ImU32 color = (i <= playIndex) ? playedColor : waveColor;
                            drawList->AddLine(ImVec2(x, yMin), ImVec2(x, yMax), color, 1.0f);
                        }
                    }
                    ImGui::EndChild();
                    ImGui::PopStyleVar();
                    ImGui::PopStyleColor();

                    ImGui::Dummy(ImVec2(0.0f, separatorSpace * 0.25f));
                    ImGui::PushStyleColor(ImGuiCol_Separator, ImVec4(0.05f, 0.05f, 0.05f, 1.0f));
                    ImGui::Separator();
                    ImGui::PopStyleColor();
                    ImGui::Dummy(ImVec2(0.0f, separatorSpace * 0.25f));

                    ImGui::BeginChild("SequencerContainer", ImVec2(0, sequencerHeight), false, ImGuiWindowFlags_NoScrollbar);

                    float msScale = 1000.0f;
                    int32_t currentFrame = static_cast<int32_t>(state.currentTime * msScale);
                    int32_t startFrame = 0;
                    int32_t endFrame = static_cast<int32_t>(state.duration * msScale);

                    ImGui::PushNeoSequencerStyleColor(ImGuiNeoSequencerCol_Bg, ImVec4(0.13f, 0.13f, 0.13f, 1.00f));
                    ImGui::PushNeoSequencerStyleColor(ImGuiNeoSequencerCol_TopBarBg, ImVec4(0.11f, 0.11f, 0.11f, 1.00f));
                    ImGui::PushNeoSequencerStyleColor(ImGuiNeoSequencerCol_FramePointer, ImVec4(0.26f, 0.59f, 0.98f, 1.00f));
                    ImGui::PushNeoSequencerStyleColor(ImGuiNeoSequencerCol_ZoomBarBg, ImVec4(0.05f, 0.05f, 0.05f, 0.54f));
                    ImGui::PushNeoSequencerStyleColor(ImGuiNeoSequencerCol_ZoomBarSlider, ImVec4(0.20f, 0.20f, 0.20f, 1.00f));
                    ImGui::PushNeoSequencerStyleColor(ImGuiNeoSequencerCol_ZoomBarSliderHovered, ImVec4(0.28f, 0.28f, 0.28f, 1.00f));
                    ImGui::PushNeoSequencerStyleColor(ImGuiNeoSequencerCol_TimelineBorder, ImVec4(0.05f, 0.05f, 0.05f, 1.0f));

                    if (ImGui::BeginNeoSequencer("AudioScrubber", &currentFrame, &startFrame, &endFrame, ImGui::GetContentRegionAvail())) {
                        ImGui::EndNeoSequencer();
                    }

                    ImGui::PopNeoSequencerStyleColor(7);

                    ImGui::EndChild();

                    float newTime = static_cast<float>(currentFrame) / msScale;
                    if (std::abs(newTime - state.currentTime) > 0.01f) {
                        vm.Dispatch(AudioViewportSeekIntent{ newTime });
                    }
                }
                else {
                    ImGui::Spacing();
                    ImGui::Indent(8.0f);
                    ImGui::TextDisabled("Select an audio file from the Hierarchy to play.");
                    ImGui::Unindent(8.0f);
                }
            }
            Syn::UI::EndCard();
        }
        ImGui::End();
        ImGui::PopStyleVar();
    }
}