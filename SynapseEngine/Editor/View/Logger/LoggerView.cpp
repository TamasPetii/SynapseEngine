#include "LoggerView.h"
#include "Editor/Manager/EditorIcons.h"
#include "Editor/Widgets/CardWidget.h"
#include "Engine/Logger/LogUtils.h"
#include <imgui.h>
#include <filesystem>
#include <algorithm>

namespace Syn {

    void LoggerView::Draw(LoggerViewModel& vm)
    {
        const LoggerState& state = vm.GetState();

        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(8, 8));
        ImGuiWindowFlags windowFlags = ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse;

        if (ImGui::Begin(SYN_ICON_TERMINAL " Output Log", nullptr, windowFlags))
        {
            auto getCardState = [this](const char* name) -> bool& {
                std::string key(name);
                if (_cardStates.find(key) == _cardStates.end()) _cardStates[key] = true;
                return _cardStates[key];
                };

            float windowBottomY = ImGui::GetWindowPos().y + ImGui::GetWindowSize().y - ImGui::GetStyle().WindowPadding.y;

            constexpr const char* CardLogTitle = "Engine Logs";
            if (Syn::UI::BeginCard(CardLogTitle, SYN_ICON_LIST, getCardState(CardLogTitle))) {

                RenderTopBar(vm, state);

                ImGui::Spacing();
                ImGui::Separator();
                ImGui::Spacing();

                float currentY = ImGui::GetCursorScreenPos().y;
                float tableHeight = windowBottomY - currentY - 8.0f;
                if (tableHeight < 100.0f) tableHeight = 100.0f;

                RenderLogTable(state, tableHeight);
            }
            Syn::UI::EndCard();
        }
        ImGui::End();
        ImGui::PopStyleVar();
    }

    void LoggerView::RenderTopBar(LoggerViewModel& vm, const LoggerState& state)
    {
        auto DrawFilterToggle = [&](const char* label, LogLevel level, bool& currentVal) {
            ImGui::PushStyleColor(ImGuiCol_Text, currentVal ? GetColorForLevel(level) : ImVec4(0.4f, 0.4f, 0.4f, 1.0f));
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 0, 0, 0));
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(1, 1, 1, 0.1f));
            ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(1, 1, 1, 0.2f));

            if (ImGui::Button(label)) {
                bool newVal = !currentVal;
                vm.Dispatch(LoggerToggleLevelIntent{ level, newVal });
            }

            ImGui::PopStyleColor(4);
            };

        bool showInfo = state.filters.showInfo;
        bool showWarn = state.filters.showWarning;
        bool showError = state.filters.showError;
        bool showCrit = state.filters.showCritical;

        DrawFilterToggle("Info", LogLevel::Info, showInfo); ImGui::SameLine();
        DrawFilterToggle("Warn", LogLevel::Warning, showWarn); ImGui::SameLine();
        DrawFilterToggle("Error", LogLevel::Error, showError); ImGui::SameLine();
        DrawFilterToggle("Crit", LogLevel::Critical, showCrit);

        ImGui::SameLine(0, 15.0f);

        ImGui::AlignTextToFramePadding();
        ImGui::TextDisabled(SYN_ICON_SEARCH);
        ImGui::SameLine();

        char searchBuffer[256];
        strncpy(searchBuffer, state.filters.searchQuery.c_str(), sizeof(searchBuffer));
        searchBuffer[sizeof(searchBuffer) - 1] = '\0';

        float rightItemsTotalWidth = ImGui::CalcTextSize("Auto-Scroll").x + ImGui::CalcTextSize(SYN_ICON_TRASH " Clear").x + 70.0f;

        ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x - rightItemsTotalWidth);
        if (ImGui::InputTextWithHint("##LogSearch", "Filter logs by message or file...", searchBuffer, IM_ARRAYSIZE(searchBuffer))) {
            vm.Dispatch(LoggerSetSearchQueryIntent{ std::string(searchBuffer) });
        }

        ImGui::SameLine();

        bool autoScroll = state.autoScroll;
        if (ImGui::Checkbox("Auto-Scroll", &autoScroll)) vm.Dispatch(LoggerSetAutoScrollIntent{ autoScroll });

        ImGui::SameLine();
        if (ImGui::Button(SYN_ICON_TRASH " Clear")) vm.Dispatch(LoggerClearIntent{});
    }

    void LoggerView::RenderLogTable(const LoggerState& state, float tableHeight) {
        ImGui::PushStyleVar(ImGuiStyleVar_CellPadding, ImVec2(4, 4));
        ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, 8.0f);
        ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.08f, 0.08f, 0.08f, 0.6f));

        ImGui::BeginChild("LogTableContainer", ImVec2(0, tableHeight), false, ImGuiWindowFlags_NoScrollbar);

        if (state.filteredLogs.empty()) {
            ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 8.0f);
            ImGui::SetCursorPosX(ImGui::GetCursorPosX() + 8.0f);
            ImGui::TextDisabled("No logs match the current filters.");
        }
        else {
            ImGuiTableFlags flags = ImGuiTableFlags_Resizable | ImGuiTableFlags_ScrollY | ImGuiTableFlags_ScrollX | ImGuiTableFlags_PadOuterX;

            if (ImGui::BeginTable("LogTable", 4, flags)) {
                ImGui::TableSetupScrollFreeze(0, 1);
                ImGui::TableSetupColumn("Time", ImGuiTableColumnFlags_WidthFixed, 100.0f);
                ImGui::TableSetupColumn("Level", ImGuiTableColumnFlags_WidthFixed, 50.0f);
                ImGui::TableSetupColumn("Source", ImGuiTableColumnFlags_WidthFixed, 130.0f);
                ImGui::TableSetupColumn("Message", ImGuiTableColumnFlags_WidthStretch);

                ImGui::TableNextRow(ImGuiTableRowFlags_Headers);
                for (int column = 0; column < 4; column++) {
                    ImGui::TableSetColumnIndex(column);
                    const char* columnName = ImGui::TableGetColumnName(column);
                    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.6f, 0.6f, 0.6f, 1.0f));
                    ImGui::TableHeader(columnName);
                    ImGui::PopStyleColor();
                }

                ImGuiListClipper clipper;
                clipper.Begin(static_cast<int>(state.filteredLogs.size()));

                while (clipper.Step()) {
                    for (int row = clipper.DisplayStart; row < clipper.DisplayEnd; row++) {
                        const auto& log = state.filteredLogs[row];

                        ImGui::TableNextRow();
                        ImGui::PushStyleColor(ImGuiCol_Text, GetColorForLevel(log.level));

                        ImGui::TableNextColumn();
                        ImGui::TextUnformatted(LogUtils::FormatTime(log.time).c_str());

                        ImGui::TableNextColumn();
                        ImGui::TextUnformatted(LogUtils::LevelToString(log.level).data());

                        ImGui::TableNextColumn();
                        std::string filenameStr = std::filesystem::path(std::string(log.file)).filename().string();
                        ImGui::Text("%s:%d", filenameStr.c_str(), log.line);

                        ImGui::TableNextColumn();

                        std::string singleLineMsg = std::string(log.message.data(), log.message.length());
                        std::replace(singleLineMsg.begin(), singleLineMsg.end(), '\n', ' ');

                        ImGui::TextUnformatted(singleLineMsg.c_str());

                        if (ImGui::IsItemHovered() && log.message.find('\n') != std::string::npos) {
                            ImGui::BeginTooltip();
                            ImGui::TextUnformatted(log.message.data());
                            ImGui::EndTooltip();
                        }

                        ImGui::PopStyleColor();
                    }
                }

                if (state.autoScroll && ImGui::GetScrollY() >= ImGui::GetScrollMaxY()) {
                    ImGui::SetScrollHereY(1.0f);
                }

                ImGui::EndTable();
            }
        }

        ImGui::EndChild();
        ImGui::PopStyleColor();
        ImGui::PopStyleVar(2);
    }

    ImVec4 LoggerView::GetColorForLevel(LogLevel level) const {
        switch (level) {
        case LogLevel::Info:     return ImVec4(0.9f, 0.9f, 0.9f, 1.0f);
        case LogLevel::Warning:  return ImVec4(0.9f, 0.7f, 0.0f, 1.0f);
        case LogLevel::Error:    return ImVec4(1.0f, 0.3f, 0.3f, 1.0f);
        case LogLevel::Critical: return ImVec4(1.0f, 0.1f, 0.1f, 1.0f);
        default:                 return ImGui::GetStyleColorVec4(ImGuiCol_Text);
        }
    }
}