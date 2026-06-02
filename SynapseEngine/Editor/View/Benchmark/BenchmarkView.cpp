#include "BenchmarkView.h"
#include "Editor/Manager/EditorIcons.h"
#include "EditorCore/ViewModels/Benchmark/BenchmarkViewModel.h"
#include <imgui.h>
#include <imgui_internal.h>
#include <format>

namespace Syn {

    void BenchmarkView::Draw(BenchmarkViewModel& vm) {
        const BenchmarkState& state = vm.GetState();

        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(8, 8));
        if (ImGui::Begin(SYN_ICON_TACHOMETER " Performance Profiler")) 
        {
            ImGui::BeginChild("TopPanel", ImVec2(0, 115), true, ImGuiWindowFlags_NoScrollbar);
            RenderTopBar(state);
            ImGui::EndChild();

            ImGui::Spacing();

            ImGui::BeginChild("BottomPanel", ImVec2(0, 0), true);

            RenderFilterBar(vm, state);

            if (ImGui::BeginTabBar("ProfilerTabs")) {
                if (ImGui::BeginTabItem(SYN_ICON_MICROCHIP " CPU Profiler")) {
                    if (state.activeTab != ProfilerTab::CPU) vm.Dispatch(BenchmarkSwitchTabIntent{ ProfilerTab::CPU });
                    RenderProfilerTable(state.cpuTimings, state.totalCpuTimeMs, state);
                    ImGui::EndTabItem();
                }

                if (ImGui::BeginTabItem(SYN_ICON_DESKTOP " GPU Profiler")) {
                    if (state.activeTab != ProfilerTab::GPU) vm.Dispatch(BenchmarkSwitchTabIntent{ ProfilerTab::GPU });
                    RenderProfilerTable(state.gpuTimings, state.totalGpuTimeMs, state);
                    ImGui::EndTabItem();
                }
                ImGui::EndTabBar();
            }
            ImGui::EndChild();
        }
        ImGui::End();
        ImGui::PopStyleVar();
    }

    void BenchmarkView::RenderTopBar(const BenchmarkState& state) {
        ImGui::TextDisabled("FPS Graph");

        std::string overlay = std::format("Cur: {:.1f} FPS | Avg: {:.1f} FPS", state.currentFps, state.averageFps);

        ImGui::PushStyleColor(ImGuiCol_PlotLines, ImVec4(0.26f, 0.59f, 0.98f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(0.0f, 0.0f, 0.0f, 0.3f));

        ImGui::PlotLines("##FPSGraph",
            state.fpsHistory.data(),
            BenchmarkState::FPS_HISTORY_SIZE,
            state.fpsHistoryOffset,
            overlay.c_str(),
            0.0f, 1500.0f,
            ImVec2(ImGui::GetContentRegionAvail().x, 50.0f)
        );
        ImGui::PopStyleColor(2);

        ImGui::Spacing();
        ImGui::Text(SYN_ICON_MICROCHIP " Global CPU Time: %.3f ms", state.totalCpuTimeMs);
        ImGui::SameLine(ImGui::GetWindowWidth() * 0.5f);
        ImGui::Text(SYN_ICON_DESKTOP " Global GPU Time: %.3f ms", state.totalGpuTimeMs);
    }

    void BenchmarkView::RenderFilterBar(BenchmarkViewModel& vm, const BenchmarkState& state) {
        ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(4, 4));

        ImGui::TextDisabled(SYN_ICON_SEARCH);
        ImGui::SameLine();

        char searchBuffer[256];
        strncpy(searchBuffer, state.filters.searchQuery.c_str(), sizeof(searchBuffer));
        searchBuffer[sizeof(searchBuffer) - 1] = '\0';

        ImGui::SetNextItemWidth(200.0f);
        if (ImGui::InputTextWithHint("##ProfilerSearch", "Filter tasks...", searchBuffer, IM_ARRAYSIZE(searchBuffer))) {
            vm.Dispatch(BenchmarkSetSearchQueryIntent{ std::string(searchBuffer) });
        }

        bool showUpdate = state.filters.showUpdate;
        if (ImGui::Checkbox(SystemPhaseNames::Update, &showUpdate)) vm.Dispatch(BenchmarkTogglePhaseFilterIntent{ SystemPhaseNames::Update, showUpdate });

        ImGui::SameLine(0, 16.0f);
        bool showUpload = state.filters.showUploadGPU;
        if (ImGui::Checkbox(SystemPhaseNames::UploadGPU, &showUpload)) vm.Dispatch(BenchmarkTogglePhaseFilterIntent{ SystemPhaseNames::UploadGPU, showUpload });

        ImGui::SameLine(0, 16.0f);
        bool showFinish = state.filters.showFinish;
        if (ImGui::Checkbox(SystemPhaseNames::Finish, &showFinish)) vm.Dispatch(BenchmarkTogglePhaseFilterIntent{ SystemPhaseNames::Finish, showFinish });

        ImGui::PopStyleVar();
        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();
    }

    void BenchmarkView::RenderProfilerTable(const std::vector<UiProfilerGroup>& timings, float totalTime, const BenchmarkState& state) {
        ImGui::PushStyleVar(ImGuiStyleVar_CellPadding, ImVec2(4, 4));
        ImGui::PushStyleVar(ImGuiStyleVar_IndentSpacing, 16.0f);

        if (ImGui::BeginTable("ProfilerTable", 3, ImGuiTableFlags_BordersInnerV | ImGuiTableFlags_RowBg | ImGuiTableFlags_Resizable | ImGuiTableFlags_ScrollY)) {

            ImGui::TableSetupScrollFreeze(0, 1);
            ImGui::TableSetupColumn("System / Pass", ImGuiTableColumnFlags_WidthStretch, 0.5f);
            ImGui::TableSetupColumn("Time (ms)", ImGuiTableColumnFlags_WidthFixed, 80.0f);
            ImGui::TableSetupColumn("Cost (%)", ImGuiTableColumnFlags_WidthStretch, 0.5f);
            ImGui::TableHeadersRow();

            for (const auto& group : timings) {
                RenderGroupRow(group, totalTime, state);
            }

            ImGui::EndTable();
        }
        ImGui::PopStyleVar(2);
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