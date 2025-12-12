#include "GraphicsDebugWindow.h"
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/quaternion.hpp>
#include "Engine/Components/TransformComponent.h"
#include "Engine/Components/CameraComponent.h"
#include "Engine/Components/PointLightComponent.h"
#include "Engine/Components/SpotLightComponent.h"
#include "Engine/Systems/CameraSystem.h"

void GraphicsDebugWindow::Render(std::shared_ptr<Registry> registry, std::shared_ptr<ResourceManager> resourceManager, std::set<VkDescriptorSet>&textureSet, uint32_t frameIndex)
{
    static Window::Config windowConfig{
        .name = "Viewport-Debug",
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

            if (ImGui::CollapsingHeader("Depth Pyramid"))
            {
                ImGui::Text("Levels: %d", levels);
                for (int i = 0; i < levels; i++)
                {
                    auto imageView = frameBuffer->GetImage("DepthPyramid")->GetImageView("Default_mip_" + std::to_string(i));
                    VkDescriptorSet image = ImGui_ImplVulkan_AddTexture(sampler, imageView, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
                    textureSet.insert(image);

                    ImGui::Image((ImTextureID)image, ImVec2(contentRegion));
                }
            }

            if (ImGui::CollapsingHeader("Bloom"))
            {
                for (int i = 0; i < levels; i++)
                {
                    auto imageView = frameBuffer->GetImage("Bloom")->GetImageView("Default_mip_" + std::to_string(i));
                    VkDescriptorSet image = ImGui_ImplVulkan_AddTexture(sampler, imageView, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
                    textureSet.insert(image);

                    ImGui::Image((ImTextureID)image, ImVec2(contentRegion));
                }
            }

            if (ImGui::CollapsingHeader("Spot Light Shadows"))
            {
                auto spotLightShadowPool = registry->GetPool<SpotLightShadowComponent>();
                if (spotLightShadowPool)
                {
                    auto& indices = spotLightShadowPool->GetDenseIndices();
                    if (indices.empty()) {
                        ImGui::Text("No Spot Light Shadows active.");
                    }

                    for (const auto& entity : indices)
                    {
                        auto& shadowComponent = spotLightShadowPool->GetData(entity);
                        auto shadowFbo = shadowComponent.frameBuffers[frameIndex].frameBuffer;

                        if (shadowFbo)
                        {
                            ImGui::PushID((int)entity);
                            ImGui::Text("Entity ID: %d", entity);

                            auto imageView = shadowFbo->GetImage("Depth")->GetImageView("Default");
                            VkDescriptorSet image = ImGui_ImplVulkan_AddTexture(sampler, imageView, VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL);
                            textureSet.insert(image);

                            float size = 128.0f;
                            ImGui::Image((ImTextureID)image, ImVec2(size, size), ImVec2(0, 0), ImVec2(1, 1), ImVec4(1, 1, 1, 1), ImVec4(1, 1, 1, 1));

                            ImGui::PopID();
                            ImGui::Separator();
                        }
                    }
                }
            }

            if (ImGui::CollapsingHeader("Point Light Shadows"))
            {
                auto pointLightShadowPool = registry->GetPool<PointLightShadowComponent>();
                if (pointLightShadowPool)
                {
                    auto& indices = pointLightShadowPool->GetDenseIndices();
                    for (const auto& entity : indices)
                    {
                        auto& shadowComponent = pointLightShadowPool->GetData(entity);
                        auto shadowFbo = shadowComponent.frameBuffers[frameIndex].frameBuffer;

                        if (shadowFbo)
                        {
                            ImGui::PushID((int)entity);
                            ImGui::Text("Entity ID: %d (Point Light)", entity);

                            const char* faceNames[] = { "+X (Right)", "-X (Left)", "+Y (Top)", "-Y (Bottom)", "+Z (Front)", "-Z (Back)" };

                            float size = 128.0f;
                            
                            for (int i = 0; i < 6; ++i)
                            {
                                if (i > 0 && (i % 3 != 0)) ImGui::SameLine();

                                ImGui::BeginGroup();
                                ImGui::Text("%s", faceNames[i]);

                                auto imageView = shadowFbo->GetImage("Depth")->GetImageView("Face_" + std::to_string(i));

                                VkDescriptorSet image = ImGui_ImplVulkan_AddTexture(sampler, imageView, VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL);
                                textureSet.insert(image);

                                ImGui::Image((ImTextureID)image, ImVec2(size, size), ImVec2(0, 0), ImVec2(1, 1), ImVec4(1, 1, 1, 1), ImVec4(1, 1, 1, 1));
                                ImGui::EndGroup();
                            }

                            ImGui::PopID();
                            ImGui::Separator();
                        }
                    }
                }
            }
        });

    ImGui::PopStyleVar();
}