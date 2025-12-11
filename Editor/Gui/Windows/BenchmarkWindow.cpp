#include "BenchmarkWindow.h"
#include "Editor/Gui/Utils/Panel.h"
#include <format>
#include <numeric>
#include <string>

std::string CleanSystemName(const char* rawName)
{
    std::string name = rawName;
    const std::string prefix = "class ";
    size_t pos = name.find(prefix);
    if (pos != std::string::npos) {
        name.erase(pos, prefix.length());
    }
    return name;
}

void BenchmarkWindow::Render(std::shared_ptr<Registry> registry, std::shared_ptr<ResourceManager> resourceManager, std::set<VkDescriptorSet>& textureSet, uint32_t frameIndex)
{
    static Window::Config windowConfig{
        .name = "Benchmark"
    };

    Window::RenderWindow(windowConfig, [&]() -> void
        {
            auto& benchmarkData = resourceManager->GetBenchmarkManager()->GetAverageBenchmarkTimes();

            double totalTime = 0.0;
            for (const auto& [type, time] : benchmarkData) totalTime += time;

            ImGui::Text("Total Frame Time");

            float windowVisibleWidth = ImGui::GetContentRegionAvail().x;
            std::string totalText = std::format("{:.4f} ms", totalTime);
            float totalTextWidth = ImGui::CalcTextSize(totalText.c_str()).x;

            ImGui::SameLine(windowVisibleWidth - totalTextWidth);

            ImVec4 totalColor = (totalTime > 16.6) ? ImVec4(1.0f, 0.3f, 0.3f, 1.0f) : ImVec4(0.4f, 1.0f, 0.4f, 1.0f);
            ImGui::TextColored(totalColor, totalText.c_str());

            ImGui::Separator();
            ImGui::Spacing();

            for (auto& [typeIndex, time] : benchmarkData)
            {
                std::string cleanName = CleanSystemName(typeIndex.name());

                ImGui::Selectable(cleanName.c_str(), false, ImGuiSelectableFlags_SpanAllColumns);

                ImGui::SameLine();

                ImVec4 timeColor = ImVec4(0.7f, 0.7f, 0.7f, 1.0f);
                if (time > 2.0) timeColor = ImVec4(1.0f, 0.6f, 0.0f, 1.0f);
                if (time > 4.0) timeColor = ImVec4(1.0f, 0.3f, 0.3f, 1.0f);

                std::string timeStr = std::format("{:.4f} ms", time);
                float timeWidth = ImGui::CalcTextSize(timeStr.c_str()).x;

                float cursorX = ImGui::GetWindowContentRegionMax().x - timeWidth;
                ImGui::SetCursorPosX(cursorX);

                ImGui::TextColored(timeColor, timeStr.c_str());
            }
        });
}