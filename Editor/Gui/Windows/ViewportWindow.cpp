#include "ViewportWindow.h"
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/quaternion.hpp>
#include "Engine/Components/TransformComponent.h"
#include "Engine/Components/CameraComponent.h"
#include "Engine/Systems/CameraSystem.h"

void ViewportWindow::Render(std::shared_ptr<Registry> registry, std::shared_ptr<ResourceManager> resourceManager, std::set<VkDescriptorSet>& textureSet, uint32_t frameIndex)
{
	static Window::Config windowConfig{
		.name = "Viewport-Window",
		.flags = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_MenuBar
	};

	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));

	Window::RenderWindow(windowConfig, [&]() -> void
		{
			auto viewPortSize = ImGui::GetContentRegionAvail();

			if (ImGui::BeginMenuBar())
			{
				GizmoConfigMenu();
				ViewportImageMenu();
				ImGui::EndMenuBar();
			}

			auto imageSize = ImGui::GetContentRegionAvail();

			uint32_t nextFrameIndex = (frameIndex + 1) % GlobalConfig::FrameConfig::framesInFlight;

			auto frameBuffer = resourceManager->GetVulkanManager()->GetFrameDependentFrameBuffer("Main", frameIndex);
			auto nextFrameBuffer = resourceManager->GetVulkanManager()->GetFrameDependentFrameBuffer("Main", nextFrameIndex);

			if (static_cast<uint32_t>(imageSize.x) != nextFrameBuffer->GetSize().width || static_cast<uint32_t>(imageSize.y) != nextFrameBuffer->GetSize().height)
				resourceManager->GetVulkanManager()->MarkFrameBufferToResize("Main", nextFrameIndex, static_cast<uint32_t>(imageSize.x), static_cast<uint32_t>(imageSize.y));

			auto sampler = resourceManager->GetVulkanManager()->GetSampler("Nearest")->Value();
			auto imageViewNames = GetViewportImageName();
			auto imageView = frameBuffer->GetImage(imageViewNames.first)->GetImageView(imageViewNames.second);

			VkDescriptorSet image = ImGui_ImplVulkan_AddTexture(sampler, imageView, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
			textureSet.insert(image);

			ImVec2 imageStart = ImVec2(ImGui::GetWindowPos().x, ImGui::GetWindowPos().y + ImGui::GetWindowSize().y - imageSize.y);
			ImVec2 imageEnd = imageSize;

			ImGui::Image((ImTextureID)image, imageSize);

			RenderGizmo(registry, imageStart, imageEnd);
			GetClickedActiveEntity(registry, frameBuffer);		
		}
	);

	ImGui::PopStyleVar();
}

std::pair<std::string, std::string> ViewportWindow::GetViewportImageName()
{
	switch (viewportImage)
	{
	case 1: return { "Position", "Default" }; break;
	case 2: return { "Color",  "Color" }; break;
	case 3: return { "Normal", "Normal"}; break;
	case 4: return { "Normal", "Roughness" }; break;
	case 5: return { "Color", "Metallic" }; break;
	default: return { "Main", "Default"};
	}
}

void ViewportWindow::GizmoConfigMenu()
{
	if (ImGui::BeginMenu("Gizmo"))
	{
		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(5.0f, 0.0f));
		if (ImGui::BeginChild("##GizmoWindow", ImVec2(255, 210), ImGuiChildFlags_AlwaysUseWindowPadding))
		{
			ImGui::SeparatorText("Operation");

			int mode = static_cast<int>(gizmoConfig.mode);
			ImGui::RadioButton("Local", &mode, static_cast<int>(ImGuizmo::LOCAL)); ImGui::SameLine();
			ImGui::RadioButton("World", &mode, static_cast<int>(ImGuizmo::WORLD));
			gizmoConfig.mode = static_cast<ImGuizmo::MODE>(mode);

			int operation = static_cast<int>(gizmoConfig.operation);
			ImGui::RadioButton("Trasnlate", &operation, static_cast<int>(ImGuizmo::TRANSLATE)); ImGui::SameLine();
			ImGui::RadioButton("Rotate", &operation, static_cast<int>(ImGuizmo::ROTATE)); ImGui::SameLine();
			ImGui::RadioButton("Scale", &operation, static_cast<int>(ImGuizmo::SCALE));
			gizmoConfig.operation = static_cast<ImGuizmo::OPERATION>(operation);

			ImGui::SeparatorText("Snapping");

			ImGui::Text("Enable"); ImGui::SameLine(80);
			ImGui::Checkbox("##EnableSnap", &gizmoConfig.useSnap);

			ImGui::Text("Translate"); ImGui::SameLine(80);
			ImGui::DragFloat3("##TranslateSnap", gizmoConfig.snapTranslate);

			ImGui::Text("Rotate"); ImGui::SameLine(80);
			ImGui::DragFloat("##RotateSnap", &gizmoConfig.snapAngle);

			ImGui::Text("Scale"); ImGui::SameLine(80);
			ImGui::DragFloat("##ScaleSnap", &gizmoConfig.snapScale);

			ImGui::EndChild();
		}
		ImGui::PopStyleVar();
		ImGui::EndMenu();
	}
}

void ViewportWindow::ViewportImageMenu()
{
	if (ImGui::BeginMenu("Image"))
	{
		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(5.0f, 0.0f));
		if (ImGui::BeginChild("##ViewportImage", ImVec2(255, 210), ImGuiChildFlags_AlwaysUseWindowPadding))
		{
			ImGui::RadioButton("Main", &viewportImage, 0);
			ImGui::RadioButton("Position", &viewportImage, 1);
			ImGui::RadioButton("Color", &viewportImage, 2);
			ImGui::RadioButton("Normal", &viewportImage, 3);
			ImGui::RadioButton("Roughness", &viewportImage, 4);
			ImGui::RadioButton("Metallic", &viewportImage, 5);

			ImGui::EndChild();
		}
		ImGui::PopStyleVar();

		ImGui::EndMenu();
	}
}

void ViewportWindow::RenderGizmo(std::shared_ptr<Registry> registry, ImVec2 viewPortStart, ImVec2 viewPortEnd)
{
	auto [cameraPool, transformPool] = registry->GetPools<CameraComponent, TransformComponent>();

	if (!cameraPool || !transformPool)
		return;

	auto cameraEntity = CameraSystem::GetMainCameraEntity(registry);
	auto& cameraComponent = cameraPool->GetData(cameraEntity);
	auto activeEntity = registry->GetActiveEntity();

	if (activeEntity != NULL_ENTITY && transformPool->HasComponent(activeEntity))
	{
		float* snapValue = nullptr;

		switch (gizmoConfig.operation) {
		case ImGuizmo::TRANSLATE:
			snapValue = gizmoConfig.snapTranslate;
			break;
		case ImGuizmo::ROTATE:
			snapValue = &gizmoConfig.snapAngle;
			break;
		case ImGuizmo::SCALE:
			snapValue = &gizmoConfig.snapScale;
			break;
		}

		glm::mat4 viewMatrix = cameraComponent.view;
		glm::mat4 projectionMatrix = cameraComponent.proj;
		auto& transformComponent = registry->GetComponent<TransformComponent>(activeEntity);

		glm::vec3 prevTranslation, prevScale, prevRotation;
		ImGuizmo::DecomposeMatrixToComponents(glm::value_ptr(transformComponent.transform), glm::value_ptr(prevTranslation), glm::value_ptr(prevRotation), glm::value_ptr(prevScale));
		glm::mat4 transform = transformComponent.transform;

		ImGuizmo::BeginFrame();
		ImGuizmo::Enable(true);
		ImGuizmo::SetOrthographic(false);
		ImGuizmo::SetRect(viewPortStart.x, viewPortStart.y, viewPortEnd.x, viewPortEnd.y);
		ImGuizmo::SetDrawlist();
		ImGuizmo::Manipulate(glm::value_ptr(viewMatrix), glm::value_ptr(projectionMatrix), gizmoConfig.operation, gizmoConfig.mode, glm::value_ptr(transform), NULL, gizmoConfig.useSnap ? snapValue : NULL, NULL, NULL);

		if (ImGuizmo::IsUsing())
		{
			auto& relationShip = registry->GetComponent<Relationship>(activeEntity);
			glm::mat4 localTransform = transform;
			if (relationShip.parent != NULL_ENTITY && registry->HasComponent<TransformComponent>(relationShip.parent))
			{
				localTransform = glm::inverse(registry->GetComponent<TransformComponent>(relationShip.parent).transform) * transform;
			}

			glm::vec3 newTranslation, newScale, newRotation;
			ImGuizmo::DecomposeMatrixToComponents(glm::value_ptr(localTransform), glm::value_ptr(newTranslation), glm::value_ptr(newRotation), glm::value_ptr(newScale));

			switch (gizmoConfig.operation)
			{
			case ImGuizmo::TRANSLATE:
				transformComponent.translation = newTranslation;
				transformPool->SetBit<UPDATE_BIT>(activeEntity);
				break;
			case ImGuizmo::ROTATE:
				transformComponent.rotation = newRotation;
				transformPool->SetBit<UPDATE_BIT>(activeEntity);
				break;
			case ImGuizmo::SCALE:
				transformComponent.scale = newScale;
				transformPool->SetBit<UPDATE_BIT>(activeEntity);
				break;
			}
		}

		if (ImGui::IsKeyPressed(ImGuiKey_1))
		{
			gizmoConfig.operation = ImGuizmo::TRANSLATE;
		}
		else if (ImGui::IsKeyPressed(ImGuiKey_2))
		{
			gizmoConfig.operation = ImGuizmo::ROTATE;
		}
		else if (ImGui::IsKeyPressed(ImGuiKey_3))
		{
			gizmoConfig.operation = ImGuizmo::SCALE;
		}
	}
}

void ViewportWindow::GetClickedActiveEntity(std::shared_ptr<Registry> registry, std::shared_ptr<Vk::FrameBuffer> frameBuffer)
{
	if (ImGui::IsMouseClicked(ImGuiMouseButton_Left) && ImGui::IsWindowHovered() && !ImGuizmo::IsUsing())
	{
		int mouseX = ImGui::GetMousePos().x - ImGui::GetWindowContentRegionMin().x - ImGui::GetWindowPos().x;
		int mouseY = ImGui::GetMousePos().y - ImGui::GetWindowContentRegionMin().y - ImGui::GetWindowPos().y;
		int contentRegionX = ImGui::GetContentRegionMax().x - ImGui::GetWindowContentRegionMin().x;
		int contentRegionY = ImGui::GetContentRegionMax().y - ImGui::GetWindowContentRegionMin().y;
		mouseY = contentRegionY - mouseY;

		if (mouseX >= 0 && mouseX < contentRegionX &&
			mouseY >= 0 && mouseY < contentRegionY)
		{
			VkDeviceSize bufferSize = sizeof(uint32_t);

			Vk::BufferConfig stagingConfig;
			stagingConfig.size = bufferSize;
			stagingConfig.usage = VK_BUFFER_USAGE_2_TRANSFER_DST_BIT;
			stagingConfig.memoryProperties = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT;
			Vk::Buffer stagingBuffer{ stagingConfig };

			Vk::VulkanContext::GetContext()->GetImmediateQueue()->SubmitTransfer(
				[&](VkCommandBuffer commandBuffer) -> void
				{
					Vk::Buffer::CopyImageToBuffer(commandBuffer, frameBuffer->GetImage("Entity")->Value(), stagingBuffer.Value(), 1, 1, mouseX, frameBuffer->GetSize().height - mouseY);
				}
			);

			uint32_t entityID;
			void* mappedBuffer = stagingBuffer.MapMemory();
			memcpy(&entityID, mappedBuffer, (size_t)bufferSize);
			stagingBuffer.UnmapMemory();

			registry->SetActiveEntity(entityID);
		}
	}
}
