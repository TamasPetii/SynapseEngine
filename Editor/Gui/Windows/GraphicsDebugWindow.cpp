#include "GraphicsDebugWindow.h"
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/quaternion.hpp>
#include "Engine/Components/TransformComponent.h"
#include "Engine/Components/CameraComponent.h"
#include "Engine/Systems/CameraSystem.h"

void GraphicsDebugWindow::Render(std::shared_ptr<Registry> registry, std::shared_ptr<ResourceManager> resourceManager, std::set<VkDescriptorSet>& textureSet, uint32_t frameIndex)
{
	static Window::Config windowConfig{
		.name = "Graphics-Debug-Window",
		.flags = ImGuiWindowFlags_NoTitleBar
	};

	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));

    Window::RenderWindow(windowConfig, [&]() -> void
        {
            auto contentRegion = ImGui::GetContentRegionAvail();

            auto frameBuffer = resourceManager->GetVulkanManager()->GetFrameDependentFrameBuffer("Main", frameIndex);
            auto sampler = resourceManager->GetVulkanManager()->GetSampler("Nearest")->Value();

            uint32_t baseWidth = frameBuffer->GetSize().width;
            uint32_t baseHeight = frameBuffer->GetSize().height;

            uint32_t levels = Vk::Image::GetMipLevels(baseWidth, baseHeight);

            float fitScale = 1.0f;
            if (baseWidth > contentRegion.x && baseWidth > 0) {
                fitScale = contentRegion.x / (float)baseWidth;
            }

            ImGui::Text("Depth Pyramid (%d levels)", levels);
            for (int i = 0; i < levels; i++)
            {
                auto imageView = frameBuffer->GetImage("DepthPyramid")->GetImageView("Default_mip_" + std::to_string(i));
                VkDescriptorSet image = ImGui_ImplVulkan_AddTexture(sampler, imageView, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
                textureSet.insert(image);

                float mipW = (float)std::max(1u, baseWidth >> i) * fitScale;
                float mipH = (float)std::max(1u, baseHeight >> i) * fitScale;

                ImGui::Image((ImTextureID)image, ImVec2(mipW, mipH));
            }

            ImGui::Separator();

            ImGui::Text("Bloom (%d levels)", levels);
            for (int i = 0; i < levels; i++)
            {
                auto imageView = frameBuffer->GetImage("Bloom")->GetImageView("Default_mip_" + std::to_string(i));
                VkDescriptorSet image = ImGui_ImplVulkan_AddTexture(sampler, imageView, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
                textureSet.insert(image);

                float mipW = (float)std::max(1u, baseWidth >> i) * fitScale;
                float mipH = (float)std::max(1u, baseHeight >> i) * fitScale;

                ImGui::Image((ImTextureID)image, ImVec2(mipW, mipH));
            }
        });

	ImGui::PopStyleVar();
}