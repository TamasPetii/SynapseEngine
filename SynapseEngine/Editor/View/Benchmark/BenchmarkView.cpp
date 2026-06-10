#include "BenchmarkView.h"
#include "Editor/Manager/EditorIcons.h"
#include "Editor/Widgets/CardWidget.h"
#include "Editor/Widgets/ToggleWidget.h"
#include <imgui.h>
#include <imgui_internal.h>
#include <format>

#include "EditorCore/ViewModels/Benchmark/BenchmarkViewModel.h"

namespace Syn {

    void BenchmarkView::Draw(BenchmarkViewModel& vm) {
        const BenchmarkState& state = vm.GetState();

        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(8, 8));

        if (ImGui::Begin(SYN_ICON_TACHOMETER " Performance Profiler"))
        {
            auto getCardState = [this](const char* name) -> bool& {
                std::string key(name);
                if (_cardStates.find(key) == _cardStates.end()) _cardStates[key] = true;
                return _cardStates[key];
                };

            float mainContentBottomY = ImGui::GetWindowPos().y + ImGui::GetWindowContentRegionMax().y;

            constexpr const char* CardOverviewTitle = "Performance Overview";
            if (Syn::UI::BeginCard(CardOverviewTitle, SYN_ICON_TACHOMETER, getCardState(CardOverviewTitle))) {
                RenderTopBar(state);
            }
            Syn::UI::EndCard();


            constexpr const char* CardTasksTitle = "Task Timings";
            if (Syn::UI::BeginCard(CardTasksTitle, SYN_ICON_MICROCHIP, getCardState(CardTasksTitle))) {

                float spacing = ImGui::GetStyle().ItemSpacing.x;
                float halfWidth = (ImGui::GetContentRegionAvail().x - spacing) * 0.5f;

                ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 4.0f);
                if (Syn::UI::ToggleButton(SYN_ICON_MICROCHIP " CPU Profiler", state.activeTab == ProfilerTab::CPU, ImVec2(halfWidth, 32.0f))) {
                    vm.Dispatch(BenchmarkSwitchTabIntent{ ProfilerTab::CPU });
                }
                ImGui::SameLine();
                if (Syn::UI::ToggleButton(SYN_ICON_DESKTOP " GPU Profiler", state.activeTab == ProfilerTab::GPU, ImVec2(halfWidth, 32.0f))) {
                    vm.Dispatch(BenchmarkSwitchTabIntent{ ProfilerTab::GPU });
                }
                ImGui::PopStyleVar();

                ImGui::Spacing();
                ImGui::Separator();
                ImGui::Spacing();

                RenderFilterBar(vm, state);

                if (state.activeTab == ProfilerTab::CPU) {
                    RenderProfilerTable(vm, state.cpuTimings, state.totalCpuTimeMs, state, mainContentBottomY);
                }
                else {
                    RenderProfilerTable(vm, state.gpuTimings, state.totalGpuTimeMs, state, mainContentBottomY);
                }
            }
            Syn::UI::EndCard();

        }
        ImGui::End();
        ImGui::PopStyleVar();
    }

    void BenchmarkView::RenderTopBar(const BenchmarkState& state) {
        std::string overlay = std::format("Cur: {:.1f} FPS | Avg: {:.1f} FPS", state.currentFps, state.averageFps);

        ImGui::PushStyleColor(ImGuiCol_PlotLines, ImVec4(0.26f, 0.59f, 0.98f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(0.0f, 0.0f, 0.0f, 0.3f));

        float plotWidth = ImGui::GetContentRegionAvail().x;
        if (plotWidth <= 0.0f) {
            plotWidth = 1.0f;
        }

        ImGui::PlotLines("##FPSGraph",
            state.fpsHistory.data(),
            BenchmarkState::FPS_HISTORY_SIZE,
            state.fpsHistoryOffset,
            overlay.c_str(),
            0.0f, 1500.0f,
            ImVec2(plotWidth, 50.0f)
        );
        ImGui::PopStyleColor(2);

        ImGui::Spacing();
        ImGui::Text(SYN_ICON_MICROCHIP " CPU Time: %.3f ms", state.totalCpuTimeMs);
        ImGui::SameLine(ImGui::GetWindowWidth() * 0.5f);
        ImGui::Text(SYN_ICON_DESKTOP " GPU Time: %.3f ms", state.totalGpuTimeMs);
    }

    void BenchmarkView::RenderFilterBar(BenchmarkViewModel& vm, const BenchmarkState& state) {
        ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(6, 6));

        ImGui::AlignTextToFramePadding();
        ImGui::TextDisabled(SYN_ICON_SEARCH);
        ImGui::SameLine();

        char searchBuffer[256];
        strncpy(searchBuffer, state.filters.searchQuery.c_str(), sizeof(searchBuffer));
        searchBuffer[sizeof(searchBuffer) - 1] = '\0';

        ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
        if (ImGui::InputTextWithHint("##ProfilerSearch", "Filter tasks...", searchBuffer, IM_ARRAYSIZE(searchBuffer))) {
            vm.Dispatch(BenchmarkSetSearchQueryIntent{ std::string(searchBuffer) });
        }

        ImGui::PopStyleVar();
        ImGui::Spacing();
    }

    void BenchmarkView::RenderProfilerTable(BenchmarkViewModel& vm, const std::vector<UiProfilerGroup>& timings, float totalTime, const BenchmarkState& state, float mainContentBottomY) {
        ImGui::PushStyleVar(ImGuiStyleVar_CellPadding, ImVec2(4, 4));
        ImGui::PushStyleVar(ImGuiStyleVar_IndentSpacing, 16.0f);

        ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.08f, 0.08f, 0.08f, 0.6f));
        ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, 8.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));

        float currentY = ImGui::GetCursorScreenPos().y;
        float tableHeight = mainContentBottomY - currentY - 12.0f;

        if (tableHeight < 100.0f)
            tableHeight = 100.0f;

        ImGui::BeginChild(
            "TableContainer",
            ImVec2(0, tableHeight),
            ImGuiChildFlags_Borders,
            ImGuiWindowFlags_NoScrollbar
        );

        if (ImGui::BeginTable("ProfilerTable", 3, ImGuiTableFlags_BordersInnerV | ImGuiTableFlags_Resizable | ImGuiTableFlags_ScrollY)) {

            ImGui::TableSetupScrollFreeze(0, 1);
            ImGui::TableSetupColumn("Pass", ImGuiTableColumnFlags_WidthStretch, 0.5f);
            ImGui::TableSetupColumn("Time (ms)", ImGuiTableColumnFlags_WidthFixed, 80.0f);
            ImGui::TableSetupColumn("Cost (%)", ImGuiTableColumnFlags_WidthStretch, 0.5f);

            ImGui::TableNextRow(ImGuiTableRowFlags_Headers);
            for (int i = 0; i < 3; ++i) {
                ImGui::TableSetColumnIndex(i);
                ImGui::PushID(i);

                const char* columnName = ImGui::TableGetColumnName(i);
                float cellWidth = ImGui::GetColumnWidth();
                float textWidth = ImGui::CalcTextSize(columnName).x;
                ImVec2 startPos = ImGui::GetCursorPos();

                ImGui::TableHeader("");

                ImGui::SetCursorPos(ImVec2(startPos.x + (cellWidth - textWidth) * 0.5f, startPos.y + 3.0f));
                ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 1.0f, 1.0f));
                ImGui::Text("%s", columnName);
                ImGui::PopStyleColor();

                if (i == 0 && state.activeTab == ProfilerTab::CPU) {
                    ImGui::SetCursorPos(ImVec2(startPos.x + cellWidth - 26.0f, startPos.y + 1.0f));

                    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 0, 0, 0));
                    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.6f, 0.6f, 0.6f, 1.0f));
                    if (ImGui::Button(SYN_ICON_FILTER)) {
                        ImGui::OpenPopup("PhaseFilterPopup");
                    }
                    ImGui::PopStyleColor(2);

                    if (ImGui::BeginPopup("PhaseFilterPopup")) {
                        ImGui::TextDisabled("Filter Phases");
                        ImGui::Separator();

                        bool showUpdate = state.filters.showUpdate;
                        if (ImGui::Checkbox("Update", &showUpdate)) vm.Dispatch(BenchmarkTogglePhaseFilterIntent{ SystemPhaseNames::Update, showUpdate });

                        bool showUpload = state.filters.showUploadGPU;
                        if (ImGui::Checkbox("Upload GPU", &showUpload)) vm.Dispatch(BenchmarkTogglePhaseFilterIntent{ SystemPhaseNames::UploadGPU, showUpload });

                        bool showFinish = state.filters.showFinish;
                        if (ImGui::Checkbox("Finish", &showFinish)) vm.Dispatch(BenchmarkTogglePhaseFilterIntent{ SystemPhaseNames::Finish, showFinish });

                        ImGui::EndPopup();
                    }
                }
                ImGui::PopID();
            }

            for (const auto& group : timings) {
                RenderGroupRow(group, totalTime, state);
            }

            ImGui::EndTable();
        }
        ImGui::EndChild();

        ImGui::PopStyleVar(4);
        ImGui::PopStyleColor();
    }

    void BenchmarkView::RenderGroupRow(const UiProfilerGroup& group, float globalTotalTime, const BenchmarkState& state) {
        ImGui::PushID(group.name.c_str());
        ImGui::TableNextRow();
        ImGui::TableNextColumn();

        ImGuiTreeNodeFlags groupFlags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_SpanFullWidth | ImGuiTreeNodeFlags_DefaultOpen;
        if (group.phases.empty()) groupFlags |= ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen;

        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.8f, 0.2f, 1.0f));
        bool groupExpanded = ImGui::TreeNodeEx("GroupNode", groupFlags, "%s", group.name.c_str());
        ImGui::PopStyleColor();

        ImGui::TableNextColumn();
        ImGui::Text("%.3f ms", group.totalTimeMs);

        ImGui::TableNextColumn();
        RenderProgressBar(group.totalTimeMs, globalTotalTime, state);

        if (groupExpanded && !group.phases.empty()) {
            for (const auto& phase : group.phases) {
                bool hasPhaseName = !phase.name.empty();
                bool phaseExpanded = true;

                if (hasPhaseName) {
                    ImGui::PushID(phase.name.c_str());
                    ImGui::TableNextRow();
                    ImGui::TableNextColumn();

                    ImGui::Indent(12.0f);
                    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.6f, 0.8f, 1.0f, 1.0f));
                    phaseExpanded = ImGui::TreeNodeEx("PhaseNode", ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_SpanFullWidth | ImGuiTreeNodeFlags_DefaultOpen, "%s", phase.name.c_str());
                    ImGui::PopStyleColor();
                    ImGui::Unindent(12.0f);

                    ImGui::TableNextColumn();
                    ImGui::Text("%.3f ms", phase.totalTimeMs);

                    ImGui::TableNextColumn();
                    RenderProgressBar(phase.totalTimeMs, globalTotalTime, state);
                }

                if (phaseExpanded) {
                    for (const auto& entry : phase.entries) {
                        ImGui::TableNextRow();
                        ImGui::TableNextColumn();

                        float indentAmt = hasPhaseName ? 24.0f : 12.0f;
                        ImGui::Indent(indentAmt);
                        ImGui::TextDisabled("%s", entry.name.c_str());
                        ImGui::Unindent(indentAmt);

                        ImGui::TableNextColumn();
                        ImGuiColorBasedOnTime(entry.timeMs, state);
                        ImGui::Text("%.3f ms", entry.timeMs);
                        ImGui::PopStyleColor();

                        ImGui::TableNextColumn();
                        RenderProgressBar(entry.timeMs, globalTotalTime, state);
                    }
                    if (hasPhaseName) ImGui::TreePop();
                }
                if (hasPhaseName) ImGui::PopID();
            }
            ImGui::TreePop();
        }
        ImGui::PopID();
    }

    void BenchmarkView::RenderProgressBar(float timeMs, float referenceTimeMs, const BenchmarkState& state) {
        float fraction = referenceTimeMs > 0.0f ? (timeMs / referenceTimeMs) : 0.0f;

        ImVec4 barColor;
        if (timeMs >= state.criticalThresholdMs) {
            barColor = ImVec4(0.9f, 0.2f, 0.2f, 1.0f);
        }
        else if (timeMs >= state.warningThresholdMs) {
            barColor = ImVec4(0.9f, 0.7f, 0.1f, 1.0f);
        }
        else {
            barColor = ImVec4(0.3f, 0.8f, 0.3f, 1.0f);
        }

        ImGui::PushStyleColor(ImGuiCol_PlotHistogram, barColor);
        ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(0.1f, 0.1f, 0.1f, 0.5f));

        ImGui::ProgressBar(fraction, ImVec2(-FLT_MIN, 16.0f), "");

        ImGui::PopStyleColor(2);
    }

    void BenchmarkView::ImGuiColorBasedOnTime(float timeMs, const BenchmarkState& state) {
        if (timeMs >= state.criticalThresholdMs) {
            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.4f, 0.4f, 1.0f));
        }
        else if (timeMs >= state.warningThresholdMs) {
            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.8f, 0.4f, 1.0f));
        }
        else {
            ImGui::PushStyleColor(ImGuiCol_Text, ImGui::GetStyleColorVec4(ImGuiCol_Text));
        }
    }
}