#include "GlobalSettingsWindow.h"
#include "Editor/Gui/Utils/Panel.h"

void GlobalSettingsWindow::Render(std::shared_ptr<Registry> registry, std::shared_ptr<ResourceManager> resourceManager, std::set<VkDescriptorSet>& textureSet, uint32_t frameIndex)
{
	static Window::Config windowConfig{
		.name = "GlobalSettings-Window"
	};

	Window::RenderWindow(windowConfig, [&]() -> void
		{
			static Panel::Config skyboxConfig{
				.name = "Skybox"
			};

			Panel::Render(skyboxConfig, [&]() -> void
				{

				}
			);

			static Panel::Config physicsConfig{
				.name = "Physics"
			};

			Panel::Render(physicsConfig, [&]() -> void
				{

				}
			);

			static Panel::Config wireframeConfig{
				.name = "Wireframe"
			};

			Panel::Render(wireframeConfig, [&]() -> void
				{
					int offset = ImGui::GetContentRegionAvail().x / 2.f;

					ImGui::Text("AABB Colliders");
					ImGui::SameLine(offset);
					ImGui::Checkbox("##AABB Colliders", &GlobalConfig::WireframeConfig::showColliderAABB);

					ImGui::Text("Obb Colliders");
					ImGui::SameLine(offset);
					ImGui::Checkbox("##Obb Colliders", &GlobalConfig::WireframeConfig::showColliderOBB);

					ImGui::Text("Sphere Colliders");
					ImGui::SameLine(offset);
					ImGui::Checkbox("##Sphere Colliders", &GlobalConfig::WireframeConfig::showColliderSphere);

					ImGui::Text("Point Light Volume");
					ImGui::SameLine(offset);
					ImGui::Checkbox("##Point Light Volume", &GlobalConfig::WireframeConfig::showPointLights);

					ImGui::Text("Spot Light Volume");
					ImGui::SameLine(offset);
					ImGui::Checkbox("##Spot Light Volume", &GlobalConfig::WireframeConfig::showSpotLights);

					ImGui::Text("Light Billboards");
					ImGui::SameLine(offset);
					ImGui::Checkbox("##Light Billboards", &GlobalConfig::WireframeConfig::showLightBillboards);

					ImGui::Separator();

					ImGui::Text("Point Project Debug");
					ImGui::SameLine(offset);
					ImGui::Checkbox("##Point Collider Project Debug", &GlobalConfig::WireframeConfig::showPointLightsProjectedAABB);

					ImGui::Text("Spot Project Debug");
					ImGui::SameLine(offset);
					ImGui::Checkbox("##Spot Collider Project Debug", &GlobalConfig::WireframeConfig::showSpotLightsProjectedAABB);
				}
			);
		}
	);
}
