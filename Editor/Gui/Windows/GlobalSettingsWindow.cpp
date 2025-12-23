#include "GlobalSettingsWindow.h"
#include "Editor/Gui/Utils/Panel.h"
#include "Engine/Render/Renderers/BloomRenderer.h"

void GlobalSettingsWindow::Render(std::shared_ptr<Registry> registry, std::shared_ptr<ResourceManager> resourceManager, std::set<VkDescriptorSet>& textureSet, uint32_t frameIndex)
{
    static Window::Config windowConfig{
        .name = "GlobalSettings"
    };

    Window::RenderWindow(windowConfig, [&]() -> void
        {
            static Panel::Config skyboxConfig{ .name = "Skybox" };
            Panel::Render(skyboxConfig, [&]() {});

            static Panel::Config physicsConfig{ .name = "Physics" };
            Panel::Render(physicsConfig, [&]() {});

            RenderWireframeSettings();
            RenderBloomSettings();
        });
}

void GlobalSettingsWindow::RenderWireframeSettings()
{
    static Panel::Config wireframeConfig{
        .name = "Wireframe"
    };

    Panel::Render(wireframeConfig, [&]() -> void
        {
            float offset = ImGui::GetContentRegionAvail().x / 2.f;

            auto DrawCheckbox = [&](const char* label, const char* id, bool* value) {
                ImGui::Text("%s", label);
                ImGui::SameLine(offset);
                ImGui::Checkbox(id, value);
                };

            DrawCheckbox("AABB Colliders", "##AABB Colliders", &GlobalConfig::WireframeConfig::showColliderAABB);
            DrawCheckbox("Obb Colliders", "##Obb Colliders", &GlobalConfig::WireframeConfig::showColliderOBB);
            DrawCheckbox("Sphere Colliders", "##Sphere Colliders", &GlobalConfig::WireframeConfig::showColliderSphere);
            DrawCheckbox("Point Light Volume", "##Point Light Volume", &GlobalConfig::WireframeConfig::showPointLights);
            DrawCheckbox("Spot Light Volume", "##Spot Light Volume", &GlobalConfig::WireframeConfig::showSpotLights);
            DrawCheckbox("Light Billboards", "##Light Billboards", &GlobalConfig::WireframeConfig::showLightBillboards);

            ImGui::Separator();

            DrawCheckbox("Point Project Debug", "##Point Collider Project Debug", &GlobalConfig::WireframeConfig::showPointLightsProjectedAABB);
            DrawCheckbox("Spot Project Debug", "##Spot Collider Project Debug", &GlobalConfig::WireframeConfig::showSpotLightsProjectedAABB);
        
			ImGui::Separator();

            DrawCheckbox("Simulate Dir Light", "##Simulate Dir Light", &GlobalConfig::DeferredConfig::simulateDirLight);
            DrawCheckbox("Simulate Point Light", "##Simulate Point Light", &GlobalConfig::DeferredConfig::simulatePointLight);
            DrawCheckbox("Simulate Spot Light", "##Simulate Spot Light", &GlobalConfig::DeferredConfig::simulateSpotLight);

            ImGui::Separator();

            DrawCheckbox("Simulate Up Sample", "##Simulate Up Sample", &GlobalConfig::BloomConfig::simulateUpSample);
            DrawCheckbox("Simulate Down Sample", "##Simulate Down Sample", &GlobalConfig::BloomConfig::simulateDownSample);

        });
}

void GlobalSettingsWindow::RenderBloomSettings()
{
    static Panel::Config bloomConfig{
        .name = "Bloom Settings"
    };

    Panel::Render(bloomConfig, [&]() -> void
        {
            float offset = ImGui::GetContentRegionAvail().x / 2.f;

            ImGui::Text("Enable Bloom");
            ImGui::SameLine(offset);
            ImGui::Checkbox("##EnableBloom", &BloomRenderer::settings.enabled);

            ImGui::Separator();

            if (!BloomRenderer::settings.enabled)
            {
                ImGui::BeginDisabled(); // Innentõl minden inaktív
            }

            ImGui::Text("Threshold");
            ImGui::SameLine(offset);
            ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
            ImGui::DragFloat("##Threshold", &BloomRenderer::settings.threshold, 0.01f, 0.0f, 10.0f);

            ImGui::Text("Knee");
            ImGui::SameLine(offset);
            ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
            ImGui::DragFloat("##Knee", &BloomRenderer::settings.knee, 0.005f, 0.0f, 1.0f);

            ImGui::Text("Upsample Radius");
            ImGui::SameLine(offset);
            ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
            ImGui::DragFloat("##UpsampleRadius", &BloomRenderer::settings.upsampleRadius, 0.0001f, 0.0f, 0.1f, "%.4f");

            ImGui::Text("Intensity");
            ImGui::SameLine(offset);
            ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
            ImGui::DragFloat("##BloomStrength", &BloomRenderer::settings.bloomStrength, 0.005f, 0.0f, 5.0f);

            ImGui::Text("Exposure");
            ImGui::SameLine(offset);
            ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
            ImGui::DragFloat("##Exposure", &BloomRenderer::settings.exposure, 0.01f, 0.0f, 10.0f);

            if (!BloomRenderer::settings.enabled)
            {
                ImGui::EndDisabled();
            }
        });
}