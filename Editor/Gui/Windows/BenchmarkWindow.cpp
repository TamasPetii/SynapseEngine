#include "BenchmarkWindow.h"
#include "Editor/Gui/Utils/Panel.h"
#include <format>
#include <numeric>
#include <string>
#include "Engine/Engine.h"



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
            // 1. Panel: System Times
            static Panel::Config systemTimeConfig{ .name = "System Times" };
            Panel::Render(systemTimeConfig, [&]() -> void
                {
                    RenderSystemTimes(resourceManager);
                });

            // 2. Panel: Render Debug
            static Panel::Config renderDebugConfig{ .name = "Render Debug" };
            Panel::Render(renderDebugConfig, [&]() -> void
                {
                    RenderGpuDebug(resourceManager, frameIndex);
                });
        });
}

void BenchmarkWindow::RenderSystemTimes(std::shared_ptr<ResourceManager> resourceManager)
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
}

void BenchmarkWindow::RenderGpuDebug(std::shared_ptr<ResourceManager> resourceManager, uint32_t frameIndex)
{
    auto stats = Engine::stats;

    // --- Main Camera ---
    if (ImGui::CollapsingHeader("Main Camera", ImGuiTreeNodeFlags_DefaultOpen))
    {
        ImGui::Text("Visible Models: %u", stats.mainModelCount);
        ImGui::Text("Visible Shapes: %u", stats.mainShapeCount);
    }

    // --- Point Lights ---
    if (ImGui::CollapsingHeader("Point Lights (Shadows)", ImGuiTreeNodeFlags_DefaultOpen))
    {
        ImGui::Text("Active Lights: %u", stats.pointLightCount);
        ImGui::Text("Shadow Casters: %u", stats.pointShadowCount);
        ImGui::Text("Shadow Objects: %u", stats.pointObjectCount);
        ImGui::Text("Phase 1 Dispatch: (%u, %u, 1)", stats.pointDispatchX, stats.pointDispatchY);

        ImGui::Separator();

        for (const auto& lightStats : stats.pointShadowDetails)
        {
            std::string label = std::format("Shadow #{} (M: {}, S: {})###PShad{}",
                lightStats.shadowIndex,
                lightStats.visibleModels.size(),
                lightStats.visibleShapes.size(),
                lightStats.shadowIndex);

            if (ImGui::TreeNode(label.c_str()))
            {
                if (!lightStats.visibleModels.empty()) {
                    ImGui::TextColored(ImVec4(0.6f, 1.0f, 0.6f, 1.0f), "MODELS");
                    for (const auto& inst : lightStats.visibleModels) {
                        if (ImGui::TreeNode(inst.name.c_str(), "%s (%u)", inst.name.c_str(), inst.count)) {
                            ImGui::TextWrapped("Indices: ["); ImGui::SameLine();
                            for (size_t k = 0; k < inst.instanceIndices.size(); ++k) {
                                ImGui::Text("%u%s", inst.instanceIndices[k], (k < inst.instanceIndices.size() - 1) ? "," : "");
                                if ((k + 1) % 10 != 0 && k != inst.instanceIndices.size() - 1) ImGui::SameLine();
                            }
                            ImGui::SameLine(); ImGui::Text("]");
                            ImGui::TreePop();
                        }
                    }
                }
                if (!lightStats.visibleShapes.empty()) {
                    ImGui::TextColored(ImVec4(0.6f, 0.8f, 1.0f, 1.0f), "SHAPES");
                    for (const auto& inst : lightStats.visibleShapes) {
                        if (ImGui::TreeNode(inst.name.c_str(), "%s (%u)", inst.name.c_str(), inst.count)) {
                            ImGui::TextWrapped("Indices: ["); ImGui::SameLine();
                            for (size_t k = 0; k < inst.instanceIndices.size(); ++k) {
                                ImGui::Text("%u%s", inst.instanceIndices[k], (k < inst.instanceIndices.size() - 1) ? "," : "");
                                if ((k + 1) % 10 != 0 && k != inst.instanceIndices.size() - 1) ImGui::SameLine();
                            }
                            ImGui::SameLine(); ImGui::Text("]");
                            ImGui::TreePop();
                        }
                    }
                }
                ImGui::TreePop();
            }
        }
    }

    // --- Spot Lights ---
    if (ImGui::CollapsingHeader("Spot Lights (Shadows)", ImGuiTreeNodeFlags_DefaultOpen))
    {
        ImGui::Text("Active Lights: %u", stats.spotLightCount);
        ImGui::Text("Shadow Casters: %u", stats.spotShadowCount);
        ImGui::Text("Shadow Objects: %u", stats.spotObjectCount);
        ImGui::Text("Phase 1 Dispatch: (%u, %u, 1)", stats.spotDispatchX, stats.spotDispatchY);

        ImGui::Separator();

        for (const auto& lightStats : stats.spotShadowDetails)
        {
            std::string label = std::format("Shadow #{} (M: {}, S: {})###SShad{}",
                lightStats.shadowIndex,
                lightStats.visibleModels.size(),
                lightStats.visibleShapes.size(),
                lightStats.shadowIndex);

            if (ImGui::TreeNode(label.c_str()))
            {
                if (!lightStats.visibleModels.empty()) {
                    ImGui::TextColored(ImVec4(0.6f, 1.0f, 0.6f, 1.0f), "MODELS");
                    for (const auto& inst : lightStats.visibleModels) {
                        if (ImGui::TreeNode(inst.name.c_str(), "%s (%u)", inst.name.c_str(), inst.count)) {
                            ImGui::TextWrapped("Indices: ["); ImGui::SameLine();
                            for (size_t k = 0; k < inst.instanceIndices.size(); ++k) {
                                ImGui::Text("%u%s", inst.instanceIndices[k], (k < inst.instanceIndices.size() - 1) ? "," : "");
                                if ((k + 1) % 10 != 0 && k != inst.instanceIndices.size() - 1) ImGui::SameLine();
                            }
                            ImGui::SameLine(); ImGui::Text("]");
                            ImGui::TreePop();
                        }
                    }
                }
                if (!lightStats.visibleShapes.empty()) {
                    ImGui::TextColored(ImVec4(0.6f, 0.8f, 1.0f, 1.0f), "SHAPES");
                    for (const auto& inst : lightStats.visibleShapes) {
                        if (ImGui::TreeNode(inst.name.c_str(), "%s (%u)", inst.name.c_str(), inst.count)) {
                            ImGui::TextWrapped("Indices: ["); ImGui::SameLine();
                            for (size_t k = 0; k < inst.instanceIndices.size(); ++k) {
                                ImGui::Text("%u%s", inst.instanceIndices[k], (k < inst.instanceIndices.size() - 1) ? "," : "");
                                if ((k + 1) % 10 != 0 && k != inst.instanceIndices.size() - 1) ImGui::SameLine();
                            }
                            ImGui::SameLine(); ImGui::Text("]");
                            ImGui::TreePop();
                        }
                    }
                }
                ImGui::TreePop();
            }
        }
    }
}
