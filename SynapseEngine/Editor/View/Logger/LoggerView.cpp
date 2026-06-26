#include "LoggerView.h"
#include "Editor/Manager/EditorIcons.h"
#include "Editor/Widgets/CardWidget.h"
#include "Engine/Logger/LogUtils.h"
#include <imgui.h>
#include <filesystem>

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
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 0, 0, 0));
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.8f, 0.8f, 0.8f, 1.0f));
        if (ImGui::Button(SYN_ICON_FILTER " Filters")) {
            ImGui::OpenPopup("LogLevelFilterPopup");
        }
        ImGui::PopStyleColor(2);

        if (ImGui::BeginPopup("LogLevelFilterPopup")) {
            ImGui::TextDisabled("Log Levels");
            ImGui::Separator();

            bool showInfo = state.filters.showInfo;
            ImGui::PushStyleColor(ImGuiCol_Text, GetColorForLevel(LogLevel::Info));
            if (ImGui::Checkbox("Info", &showInfo)) vm.Dispatch(LoggerToggleLevelIntent{ LogLevel::Info, showInfo });
            ImGui::PopStyleColor();

            bool showWarn = state.filters.showWarning;
            ImGui::PushStyleColor(ImGuiCol_Text, GetColorForLevel(LogLevel::Warning));
            if (ImGui::Checkbox("Warning", &showWarn)) vm.Dispatch(LoggerToggleLevelIntent{ LogLevel::Warning, showWarn });
            ImGui::PopStyleColor();

            bool showError = state.filters.showError;
            ImGui::PushStyleColor(ImGuiCol_Text, GetColorForLevel(LogLevel::Error));
            if (ImGui::Checkbox("Error", &showError)) vm.Dispatch(LoggerToggleLevelIntent{ LogLevel::Error, showError });
            ImGui::PopStyleColor();

            bool showCrit = state.filters.showCritical;
            ImGui::PushStyleColor(ImGuiCol_Text, GetColorForLevel(LogLevel::Critical));
            if (ImGui::Checkbox("Critical", &showCrit)) vm.Dispatch(LoggerToggleLevelIntent{ LogLevel::Critical, showCrit });
            ImGui::PopStyleColor();

            ImGui::EndPopup();
        }

        ImGui::SameLine();

        float autoScrollWidth = ImGui::CalcTextSize("Auto-Scroll").x + 35.0f;
        float clearBtnWidth = ImGui::CalcTextSize(SYN_ICON_TRASH " Clear").x + ImGui::GetStyle().FramePadding.x * 2.0f;
        float spacing = ImGui::GetStyle().ItemSpacing.x;
        float rightItemsTotalWidth = autoScrollWidth + clearBtnWidth + spacing * 3.0f;

        ImGui::AlignTextToFramePadding();
        ImGui::TextDisabled(SYN_ICON_SEARCH);
        ImGui::SameLine();

        char searchBuffer[256];
        strncpy(searchBuffer, state.filters.searchQuery.c_str(), sizeof(searchBuffer));
        searchBuffer[sizeof(searchBuffer) - 1] = '\0';

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
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));

        ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.08f, 0.08f, 0.08f, 0.6f));

        ImGui::BeginChild("LogTableContainer", ImVec2(0, tableHeight), ImGuiChildFlags_Borders, ImGuiWindowFlags_NoScrollbar);

        if (state.filteredLogs.empty()) {
            ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 8.0f);
            ImGui::SetCursorPosX(ImGui::GetCursorPosX() + 8.0f);
            ImGui::TextDisabled("No logs match the current filters.");
        }
        else {
            ImGuiTableFlags flags = ImGuiTableFlags_BordersInnerV | ImGuiTableFlags_Resizable | ImGuiTableFlags_ScrollY | ImGuiTableFlags_RowBg;

            if (ImGui::BeginTable("LogTable", 4, flags)) {
                ImGui::TableSetupScrollFreeze(0, 1);
                ImGui::TableSetupColumn("Time", ImGuiTableColumnFlags_WidthFixed, 140.0f);
                ImGui::TableSetupColumn("Level", ImGuiTableColumnFlags_WidthFixed, 60.0f);
                ImGui::TableSetupColumn("Source", ImGuiTableColumnFlags_WidthFixed, 150.0f);
                ImGui::TableSetupColumn("Message", ImGuiTableColumnFlags_WidthStretch);

                ImGui::TableNextRow(ImGuiTableRowFlags_Headers);
                for (int column = 0; column < 4; column++) {
                    ImGui::TableSetColumnIndex(column);
                    const char* columnName = ImGui::TableGetColumnName(column);

                    ImGui::PushID(column);

                    float cellWidth = ImGui::GetColumnWidth();
                    float textWidth = ImGui::CalcTextSize(columnName).x;
                    ImVec2 startPos = ImGui::GetCursorPos();

                    ImGui::TableHeader("");

                    ImGui::SetCursorPos(ImVec2(startPos.x + (cellWidth - textWidth) * 0.5f, startPos.y + 3.0f));
                    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 1.0f, 1.0f));
                    ImGui::Text("%s", columnName);
                    ImGui::PopStyleColor();

                    ImGui::PopID();
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
                        ImGui::TextWrapped("%.*s", static_cast<int>(log.message.length()), log.message.data());

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
        ImGui::PopStyleVar(3);
    }

    ImVec4 LoggerView::GetColorForLevel(LogLevel level) const {
        switch (level) {
        case LogLevel::Info:     return ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
        case LogLevel::Warning:  return ImVec4(0.8f, 0.4f, 1.0f, 1.0f);
        case LogLevel::Error:    return ImVec4(1.0f, 0.2f, 0.2f, 1.0f);
        case LogLevel::Critical: return ImVec4(1.0f, 0.6f, 0.0f, 1.0f);
        default:                 return ImGui::GetStyleColorVec4(ImGuiCol_Text);
        }
    }
}