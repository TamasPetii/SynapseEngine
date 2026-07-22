#include "StatisticsView.h"
#include "Editor/Manager/EditorIcons.h"
#include "Editor/Widgets/CardWidget.h"
#include "Editor/Widgets/ToggleWidget.h"
#include <imgui.h>
#include <imgui_internal.h>
#include <format>

namespace Syn
{
    void StatisticsView::Draw(StatisticsViewModel& vm) {
        const StatisticsState& state = vm.GetState();

        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(8, 8));

        if (ImGui::Begin(SYN_ICON_TACHOMETER " Render Statistics"))
        {
            float spacing = ImGui::GetStyle().ItemSpacing.x;
            float tabWidth = (ImGui::GetContentRegionAvail().x - spacing * 3.0f) * 0.25f;

            ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 4.0f);
            if (Syn::UI::ToggleButton(SYN_ICON_GLOBE " Scene", state.activeTab == StatCategory::Scene, ImVec2(tabWidth, 32.0f))) {
                vm.Dispatch(StatisticsSwitchTabIntent{ StatCategory::Scene });
            }
            ImGui::SameLine();
            if (Syn::UI::ToggleButton(SYN_ICON_SUN " Dir. Shadow", state.activeTab == StatCategory::DirectionalShadow, ImVec2(tabWidth, 32.0f))) {
                vm.Dispatch(StatisticsSwitchTabIntent{ StatCategory::DirectionalShadow });
            }
            ImGui::SameLine();
            if (Syn::UI::ToggleButton(SYN_ICON_LIGHTBULB " Point Shadow", state.activeTab == StatCategory::PointShadow, ImVec2(tabWidth, 32.0f))) {
                vm.Dispatch(StatisticsSwitchTabIntent{ StatCategory::PointShadow });
            }
            ImGui::SameLine();
            if (Syn::UI::ToggleButton(SYN_ICON_SPOTLIGHT " Spot Shadow", state.activeTab == StatCategory::SpotShadow, ImVec2(tabWidth, 32.0f))) {
                vm.Dispatch(StatisticsSwitchTabIntent{ StatCategory::SpotShadow });
            }
            ImGui::PopStyleVar();

            ImGui::Spacing();
            ImGui::Separator();
            ImGui::Spacing();

            auto getCardState = [this](const char* name) -> bool& {
                std::string key(name);
                if (_cardStates.find(key) == _cardStates.end()) _cardStates[key] = true;
                return _cardStates[key];
                };

            float mainContentBottomY = ImGui::GetWindowPos().y + ImGui::GetWindowContentRegionMax().y;

            constexpr const char* CardOverviewTitle = "Aggregated Statistics";
            if (Syn::UI::BeginCard(CardOverviewTitle, SYN_ICON_TACHOMETER, getCardState(CardOverviewTitle))) {
                RenderOverviewCard(state);
            }
            Syn::UI::EndCard();

            constexpr const char* CardTasksTitle = "Render Pass Statistics";
            if (Syn::UI::BeginCard(CardTasksTitle, SYN_ICON_MICROCHIP, getCardState(CardTasksTitle))) {
                RenderPassStatsCard(vm, state, mainContentBottomY);
            }
            Syn::UI::EndCard();
        }
        ImGui::End();
        ImGui::PopStyleVar();
    }

    void StatisticsView::RenderOverviewCard(const StatisticsState& state) {
        ImGui::Spacing();

        float meshletDrawPercent = state.cpuStats.totalMaxMeshlets > 0 ? (static_cast<float>(state.totalMSInvocations) / state.cpuStats.totalMaxMeshlets) : 0.0f;
        float vertexEfficiency = state.cpuStats.maxPossibleVertices > 0 ? (static_cast<float>(state.totalVSInvocations) / state.cpuStats.maxPossibleVertices) : 0.0f;

        ImGui::PushStyleColor(ImGuiCol_PlotHistogram, ImVec4(0.2f, 0.7f, 0.3f, 1.0f));
        ImGui::ProgressBar(meshletDrawPercent, ImVec2(-1.0f, 20.0f), std::format("Meshlets: {:.1f}% Submitted (Rest Culled)", meshletDrawPercent * 100.0f).c_str());

        ImGui::PushStyleColor(ImGuiCol_PlotHistogram, ImVec4(0.8f, 0.4f, 0.2f, 1.0f));
        ImGui::ProgressBar(vertexEfficiency, ImVec2(-1.0f, 20.0f), std::format("Vertices: {:.1f}% Shaded (Rest Culled)", vertexEfficiency * 100.0f).c_str());
        ImGui::PopStyleColor(2);

        ImGui::Spacing();
        ImGui::Spacing();

        auto DrawStatRow = [](const char* label, const std::string& value) {
            ImGui::Text("%s: ", label);
            ImGui::SameLine();
            ImGui::TextColored(ImVec4(1.0f, 0.8f, 0.4f, 1.0f), "%s", value.c_str());
            };

        auto DrawSectionHeader = [](const char* title) {
            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.6f, 0.8f, 1.0f, 1.0f));
            ImGui::TextUnformatted(title);
            ImGui::PopStyleColor();
            ImGui::Separator();
            ImGui::Spacing();
            };

        DrawSectionHeader(SYN_ICON_LIGHTBULB " Lights & Shadows");
        DrawStatRow("Total Lights in Scene", std::to_string(state.cpuStats.totalLights));
        DrawStatRow("Visible Lights (Forward+)", std::to_string(state.cpuStats.visibleLights));
        DrawStatRow("Visible Shadow-Casting Lights", std::to_string(state.cpuStats.visibleShadowLights));

        if (state.activeTab != StatCategory::Scene) {
            DrawStatRow("Shadow Instances Appended", std::to_string(state.cpuStats.appendedInstances));
        }

        ImGui::Spacing();

        DrawSectionHeader(SYN_ICON_MICROCHIP " CPU Submissions");
        DrawStatRow("Total Models (Blueprints)", std::to_string(state.cpuStats.totalModels));
        DrawStatRow("Total Allocated Instances", std::to_string(state.cpuStats.totalAllocatedInstances));
        DrawStatRow("Draw Descriptors (Trad / Meshlet)", std::format("{} / {}", state.cpuStats.traditionalDrawDescriptors, state.cpuStats.meshletDrawDescriptors));

        ImGui::Spacing();

        DrawSectionHeader(SYN_ICON_CUBES " Geometry");
        DrawStatRow("Vertices (Assembled / Max)", std::format("{} / {}", state.totalInputVertices, state.cpuStats.maxPossibleVertices));
        DrawStatRow("Primitives (Assembled / Max)", std::format("{} / {}", state.totalInputPrimitives, state.cpuStats.maxPossibleTriangles));

        ImGui::Spacing();

        DrawSectionHeader(SYN_ICON_DESKTOP " Shader Invocations");
        DrawStatRow("Vertex Shaders", std::to_string(state.totalVSInvocations));
        DrawStatRow("Fragment Shaders", std::to_string(state.totalFSInvocations));
        DrawStatRow("Mesh / Task Shaders", std::format("{} / {}", state.totalMSInvocations, state.totalTSInvocations));

        ImGui::Spacing();

        DrawSectionHeader(SYN_ICON_CUT " Clipping Stage");
        DrawStatRow("Primitives Reaching Clipper", std::to_string(state.totalClippingInvocations));
        DrawStatRow("Primitives Passed to Rasterizer", std::to_string(state.totalClippingPrimitives));

        int64_t clippedDifference = static_cast<int64_t>(state.totalClippingInvocations) - static_cast<int64_t>(state.totalClippingPrimitives);
        std::string cullText = clippedDifference >= 0 ? std::to_string(clippedDifference) + " (Culled)" : std::to_string(std::abs(clippedDifference)) + " (Added via Cut)";
        DrawStatRow("Net Primitives Difference", cullText);

        ImGui::Spacing();
    }

    void StatisticsView::RenderPassStatsCard(StatisticsViewModel& vm, const StatisticsState& state, float mainContentBottomY) {
        ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(6, 6));
        ImGui::AlignTextToFramePadding();
        ImGui::TextDisabled(SYN_ICON_SEARCH);
        ImGui::SameLine();

        char searchBuffer[256];
        strncpy(searchBuffer, state.searchQuery.c_str(), sizeof(searchBuffer));
        searchBuffer[sizeof(searchBuffer) - 1] = '\0';

        ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
        if (ImGui::InputTextWithHint("##StatsSearch", "Filter render passes...", searchBuffer, IM_ARRAYSIZE(searchBuffer))) {
            vm.Dispatch(StatisticsSetSearchQueryIntent{ std::string(searchBuffer) });
        }
        ImGui::PopStyleVar();
        ImGui::Spacing();

        ImGui::PushStyleVar(ImGuiStyleVar_CellPadding, ImVec2(4, 4));
        ImGui::PushStyleVar(ImGuiStyleVar_IndentSpacing, 16.0f);
        ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.08f, 0.08f, 0.08f, 0.6f));
        ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, 8.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));

        float currentY = ImGui::GetCursorScreenPos().y;
        float tableHeight = mainContentBottomY - currentY - 12.0f;
        if (tableHeight < 150.0f) tableHeight = 150.0f;

        ImGui::BeginChild("StatsTableContainer", ImVec2(0, tableHeight), ImGuiChildFlags_Borders, ImGuiWindowFlags_NoScrollbar);

        if (ImGui::BeginTable("GpuStatsTable", 7, ImGuiTableFlags_BordersInnerV | ImGuiTableFlags_Resizable | ImGuiTableFlags_ScrollY)) {
            ImGui::TableSetupScrollFreeze(0, 1);
            ImGui::TableSetupColumn("Group / Pass", ImGuiTableColumnFlags_WidthStretch, 2.0f);
            ImGui::TableSetupColumn("Vertices", ImGuiTableColumnFlags_WidthFixed, 80.0f);
            ImGui::TableSetupColumn("Primitives", ImGuiTableColumnFlags_WidthFixed, 80.0f);
            ImGui::TableSetupColumn("VS Invoc", ImGuiTableColumnFlags_WidthFixed, 80.0f);
            ImGui::TableSetupColumn("Clip In/Out", ImGuiTableColumnFlags_WidthFixed, 100.0f);
            ImGui::TableSetupColumn("FS Invoc", ImGuiTableColumnFlags_WidthFixed, 80.0f);
            ImGui::TableSetupColumn("MS/TS Invoc", ImGuiTableColumnFlags_WidthFixed, 100.0f);

            ImGui::TableNextRow(ImGuiTableRowFlags_Headers);
            for (int i = 0; i < 7; ++i) {
                ImGui::TableSetColumnIndex(i);
                ImGui::PushID(i);
                const char* columnName = ImGui::TableGetColumnName(i);
                float cellWidth = ImGui::GetColumnWidth();
                float textWidth = ImGui::CalcTextSize(columnName).x;
                ImVec2 startPos = ImGui::GetCursorPos();

                ImGui::TableHeader("");

                ImGui::SetCursorPos(ImVec2(startPos.x + (cellWidth - textWidth) * 0.5f, startPos.y + 3.0f));
                ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 1.0f, 1.0f));
                ImGui::TextUnformatted(columnName);
                ImGui::PopStyleColor();
                ImGui::PopID();
            }

            for (const auto& group : state.groupedGpuStats) {
                RenderGroupRow(group);
            }

            ImGui::EndTable();
        }
        ImGui::EndChild();

        ImGui::PopStyleVar(4);
        ImGui::PopStyleColor();
    }

    void StatisticsView::RenderGroupRow(const UiStatGroup& group) {
        ImGui::PushID(group.name.c_str());
        ImGui::TableNextRow();
        ImGui::TableNextColumn();

        ImGuiTreeNodeFlags groupFlags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_SpanFullWidth | ImGuiTreeNodeFlags_DefaultOpen;
        if (group.passes.empty()) groupFlags |= ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen;

        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.8f, 0.2f, 1.0f));

        bool groupExpanded = ImGui::TreeNodeEx("GroupNode", groupFlags, "[%s]", group.name.c_str());

        ImGui::TableNextColumn(); ImGui::Text("%llu", group.inputAssemblyVertices);
        ImGui::TableNextColumn(); ImGui::Text("%llu", group.inputAssemblyPrimitives);
        ImGui::TableNextColumn(); ImGui::Text("%llu", group.vertexShaderInvocations);
        ImGui::TableNextColumn(); ImGui::Text("%llu / %llu", group.clippingInvocations, group.clippingPrimitives);
        ImGui::TableNextColumn(); ImGui::Text("%llu", group.fragmentShaderInvocations);
        ImGui::TableNextColumn(); ImGui::Text("%llu / %llu", group.meshShaderInvocations, group.taskShaderInvocations);

        ImGui::PopStyleColor();

        if (groupExpanded && !group.passes.empty()) {
            for (const auto& pass : group.passes) {
                ImGui::TableNextRow();
                ImGui::TableNextColumn();

                ImGui::Indent(16.0f);
                ImGui::TextDisabled("%s", pass.passName.c_str());
                ImGui::Unindent(16.0f);

                ImGui::TableNextColumn(); ImGui::Text("%llu", pass.inputAssemblyVertices);
                ImGui::TableNextColumn(); ImGui::Text("%llu", pass.inputAssemblyPrimitives);
                ImGui::TableNextColumn(); ImGui::Text("%llu", pass.vertexShaderInvocations);
                ImGui::TableNextColumn(); ImGui::Text("%llu / %llu", pass.clippingInvocations, pass.clippingPrimitives);
                ImGui::TableNextColumn(); ImGui::Text("%llu", pass.fragmentShaderInvocations);
                ImGui::TableNextColumn(); ImGui::Text("%llu / %llu", pass.meshShaderInvocations, pass.taskShaderInvocations);
            }
            ImGui::TreePop();
        }
        ImGui::PopID();
    }
}