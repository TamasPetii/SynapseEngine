#include "OcclusionCuller.h"
#include "Engine/Components/PointLightComponent.h"
#include "Engine/Components/SpotLightComponent.h"
#include "Engine/Systems/InstanceSystem.h"
#include "Engine/Render/GpuStructs.h"

void OcclusionCuller::CullLights(std::shared_ptr<Registry> registry, std::shared_ptr<ResourceManager> resourceManager, uint32_t frameIndex)
{
	Vk::VulkanContext::GetContext()->GetImmediateQueue()->SubmitGraphics(
		[&](VkCommandBuffer commandBuffer) -> void {
			RenderPointLightsOcclusion(commandBuffer, registry, resourceManager, frameIndex);
			RenderSpotLightsOcclusion(commandBuffer, registry, resourceManager, frameIndex);
		}
	);

	InstanceSystem::UpdatePointLightInstancesWithOcclusion(registry, resourceManager, frameIndex);
	InstanceSystem::UpdateSpotLightInstancesWithOcclusion(registry, resourceManager, frameIndex);
}

void OcclusionCuller::RenderPointLightsOcclusion(VkCommandBuffer commandBuffer, std::shared_ptr<Registry> registry, std::shared_ptr<ResourceManager> resourceManager, uint32_t frameIndex)
{
	auto pointLightPool = registry->GetPool<PointLightComponent>();

	if (!pointLightPool || pointLightPool->GetDenseSize() == 0 || PointLightComponent::instanceCount == 0)
		return;

	auto vulkanContext = Vk::VulkanContext::GetContext();
	auto device = vulkanContext->GetDevice();

	auto graphicsQueue = device->GetQueue(Vk::QueueType::GRAPHICS);

	auto frameBuffer = resourceManager->GetVulkanManager()->GetFrameDependentFrameBuffer("Main", frameIndex);
	auto pipeline = resourceManager->GetVulkanManager()->GetGraphicsPipeline("OcclusionCulling");

	VkRenderingAttachmentInfo colorAttachment = Vk::DynamicRendering::BuildRenderingAttachmentInfo(frameBuffer->GetImage("Main")->GetImageView(), VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, nullptr);
	std::vector<VkRenderingAttachmentInfo> renderTargetAttachments = { colorAttachment };

	VkRenderingAttachmentInfo depthAttachment = Vk::DynamicRendering::BuildRenderingAttachmentInfo(frameBuffer->GetImage("Depth")->GetImageView(), VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL, nullptr);
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

	auto shape = resourceManager->GetGeometryManager()->GetShape("Cube");

	OcclusionCullingPushConstants pushConstants;
	pushConstants.cameraIndex = 0; //TODO: MAIN CAMERA
	pushConstants.cameraBuffer = resourceManager->GetComponentBufferManager()->GetComponentBuffer("CameraData", frameIndex)->buffer->GetAddress();
	pushConstants.transformBufferAddress = resourceManager->GetComponentBufferManager()->GetComponentBuffer("PointLightTransform", frameIndex)->buffer->GetAddress();
	pushConstants.instanceBufferAddress = resourceManager->GetComponentBufferManager()->GetComponentBuffer("PointLightInstanceIndices", frameIndex)->buffer->GetAddress();
	pushConstants.vertexBufferAddress = shape->GetVertexBuffer()->GetAddress();
	pushConstants.indexBufferAddress = shape->GetIndexBuffer()->GetAddress();
	pushConstants.occlusionBufferAddress = resourceManager->GetComponentBufferManager()->GetComponentBuffer("PointLightOcclusionIndices", frameIndex)->buffer->GetAddress();

	vkCmdPushConstants(commandBuffer, pipeline->GetLayout(), VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(OcclusionCullingPushConstants), &pushConstants);
	vkCmdDraw(commandBuffer, shape->GetIndexCount(), PointLightComponent::instanceCount, 0, 0);
	vkCmdEndRendering(commandBuffer);
}

void OcclusionCuller::RenderSpotLightsOcclusion(VkCommandBuffer commandBuffer, std::shared_ptr<Registry> registry, std::shared_ptr<ResourceManager> resourceManager, uint32_t frameIndex)
{
	auto spotLightPool = registry->GetPool<SpotLightComponent>();

	if (!spotLightPool || spotLightPool->GetDenseSize() == 0 || SpotLightComponent::instanceCount == 0)
		return;

	auto vulkanContext = Vk::VulkanContext::GetContext();
	auto device = vulkanContext->GetDevice();

	auto graphicsQueue = device->GetQueue(Vk::QueueType::GRAPHICS);

	auto frameBuffer = resourceManager->GetVulkanManager()->GetFrameDependentFrameBuffer("Main", frameIndex);
	auto pipeline = resourceManager->GetVulkanManager()->GetGraphicsPipeline("OcclusionCulling");

	VkRenderingAttachmentInfo colorAttachment = Vk::DynamicRendering::BuildRenderingAttachmentInfo(frameBuffer->GetImage("Main")->GetImageView(), VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, nullptr);
	std::vector<VkRenderingAttachmentInfo> renderTargetAttachments = { colorAttachment };

	VkRenderingAttachmentInfo depthAttachment = Vk::DynamicRendering::BuildRenderingAttachmentInfo(frameBuffer->GetImage("Depth")->GetImageView(), VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL, nullptr);
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

	auto shape = resourceManager->GetGeometryManager()->GetShape("Cone");

	OcclusionCullingPushConstants pushConstants;
	pushConstants.cameraIndex = 0; //TODO: MAIN CAMERA
	pushConstants.cameraBuffer = resourceManager->GetComponentBufferManager()->GetComponentBuffer("CameraData", frameIndex)->buffer->GetAddress();
	pushConstants.transformBufferAddress = resourceManager->GetComponentBufferManager()->GetComponentBuffer("SpotLightTransform", frameIndex)->buffer->GetAddress();
	pushConstants.instanceBufferAddress = resourceManager->GetComponentBufferManager()->GetComponentBuffer("SpotLightInstanceIndices", frameIndex)->buffer->GetAddress();
	pushConstants.vertexBufferAddress = shape->GetVertexBuffer()->GetAddress();
	pushConstants.indexBufferAddress = shape->GetIndexBuffer()->GetAddress();
	pushConstants.occlusionBufferAddress = resourceManager->GetComponentBufferManager()->GetComponentBuffer("SpotLightOcclusionIndices", frameIndex)->buffer->GetAddress();

	vkCmdPushConstants(commandBuffer, pipeline->GetLayout(), VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(OcclusionCullingPushConstants), &pushConstants);
	vkCmdDraw(commandBuffer, shape->GetIndexCount(), SpotLightComponent::instanceCount, 0, 0);
	vkCmdEndRendering(commandBuffer);
}
