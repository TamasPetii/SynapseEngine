#include "BillboardRenderer.h"
#include "../GpuStructs.h"
#include <algorithm>
#include <execution>
#include "Engine/Components/DirectionLightComponent.h"
#include "Engine/Components/PointLightComponent.h"
#include "Engine/Components/SpotLightComponent.h"

void BillboardRenderer::Render(VkCommandBuffer commandBuffer, std::shared_ptr<Registry> registry, std::shared_ptr<ResourceManager> resourceManager, uint32_t frameIndex)
{
	auto vulkanContext = Vk::VulkanContext::GetContext();
	auto device = vulkanContext->GetDevice();
	auto graphicsQueue = device->GetQueue(Vk::QueueType::GRAPHICS);
	auto frameBuffer = resourceManager->GetVulkanManager()->GetFrameDependentFrameBuffer("Main", frameIndex);
	auto pipeline = resourceManager->GetVulkanManager()->GetGraphicsPipeline("Billboard");

	VkRenderingAttachmentInfo colorAttachment = Vk::DynamicRendering::BuildRenderingAttachmentInfo(frameBuffer->GetImage("Main")->GetImageView(), VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, nullptr);
	VkRenderingAttachmentInfo entityAttachment = Vk::DynamicRendering::BuildRenderingAttachmentInfo(frameBuffer->GetImage("Entity")->GetImageView(), VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, nullptr);
	VkRenderingAttachmentInfo depthAttachment = Vk::DynamicRendering::BuildRenderingAttachmentInfo(frameBuffer->GetImage("Depth")->GetImageView(), VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL, nullptr);

	std::vector<VkRenderingAttachmentInfo> renderTargetAttachments = { colorAttachment, entityAttachment };
	VkRenderingInfo renderingInfo = Vk::DynamicRendering::BuildRenderingInfo(frameBuffer->GetSize(), renderTargetAttachments, &depthAttachment);

	vkCmdBeginRendering(commandBuffer, &renderingInfo);
	vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline->GetPipeline());

	VkViewport viewport{};
	viewport.x = 0.0f;
	viewport.y = 0.0f;
	viewport.width = (float)frameBuffer->GetSize().width;
	viewport.height = (float)frameBuffer->GetSize().height;
	viewport.minDepth = 0.0f;
	viewport.maxDepth = 1.0f;
	vkCmdSetViewport(commandBuffer, 0, 1, &viewport);

	VkRect2D scissor{};
	scissor.offset = { 0, 0 };
	scissor.extent = frameBuffer->GetSize();
	vkCmdSetScissor(commandBuffer, 0, 1, &scissor);

	auto textureDescriptorSet = resourceManager->GetVulkanManager()->GetDescriptorSet("LoadedImages");
	vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline->GetLayout(), 0, 1, &textureDescriptorSet->Value(), 0, nullptr);

	auto RenderBillboard = [&](const std::string& billboardBufferName, const std::string& instanceBufferName, uint32_t instanceCount) -> void
		{
			if (instanceCount > 0)
			{
				BillboardPushConstants pushConstants;
				pushConstants.cameraIndex = 0;
				pushConstants.cameraBuffer = resourceManager->GetComponentBufferManager()->GetComponentBuffer("CameraData", frameIndex)->buffer->GetAddress();
				pushConstants.positionBufferAddress = resourceManager->GetComponentBufferManager()->GetComponentBuffer(billboardBufferName, frameIndex)->buffer->GetAddress();
				pushConstants.instanceBufferAddress = resourceManager->GetComponentBufferManager()->GetComponentBuffer(instanceBufferName, frameIndex)->buffer->GetAddress();
				pushConstants.iconIndex = 0;

				vkCmdPushConstants(commandBuffer, pipeline->GetLayout(), VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_GEOMETRY_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(BillboardPushConstants), &pushConstants);
				vkCmdDraw(commandBuffer, instanceCount, 1, 0, 0);
			}
		};

	/*
	if (auto directionLightPool = registry->GetPool<DirectionLightComponent>())
	{
		uint32_t count = directionLightPool->GetDenseSize();

		if (GlobalConfig::BillboardConfig::showPointLights)
			RenderBillboard("DirectionLightBillboard", "PointLightTransform", count);
	}
	*/

	if (auto pointLightPool = registry->GetPool<PointLightComponent>())
	{
		uint32_t count = pointLightPool->GetDenseSize();

		if (GlobalConfig::BillboardConfig::showPointLights)
			RenderBillboard("PointLightBillboard", "PointLightInstanceIndices", count);
	}

	if (auto spotLightPool = registry->GetPool<SpotLightComponent>())
	{
		uint32_t count = spotLightPool->GetDenseSize();

		if (GlobalConfig::BillboardConfig::showSpotLights)
			RenderBillboard("SpotLightBillboard", "SpotLightInstanceIndices", count);
	}

	vkCmdEndRendering(commandBuffer);
}