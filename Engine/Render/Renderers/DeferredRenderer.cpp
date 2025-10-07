#include "DeferredRenderer.h"
#include "Engine/Components/DirectionLightComponent.h"
#include "Engine/Components/PointLightComponent.h"
#include "Engine/Components/SpotLightComponent.h"
#include "Engine/Render/GpuStructs.h"

void DeferredRenderer::Initialize(std::shared_ptr<ResourceManager> resourceManager)
{
}

void DeferredRenderer::Render(VkCommandBuffer commandBuffer, std::shared_ptr<Registry> registry, std::shared_ptr<ResourceManager> resourceManager, uint32_t frameIndex, std::function<void()> renderFunction)
{
	auto frameBuffer = resourceManager->GetVulkanManager()->GetFrameDependentFrameBuffer("Main", frameIndex);

	Vk::Image::TransitionImageLayoutDynamic(commandBuffer, frameBuffer->GetImage("Position")->Value(),
		VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT, VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT,
		VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT, VK_ACCESS_2_SHADER_READ_BIT);

	Vk::Image::TransitionImageLayoutDynamic(commandBuffer, frameBuffer->GetImage("Color")->Value(),
		VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT, VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT,
		VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT, VK_ACCESS_2_SHADER_READ_BIT);

	Vk::Image::TransitionImageLayoutDynamic(commandBuffer, frameBuffer->GetImage("Normal")->Value(),
		VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT, VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT,
		VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT, VK_ACCESS_2_SHADER_READ_BIT);

	VkClearValue clearValue;
	clearValue.color.float32[0] = 0.f;
	clearValue.color.float32[1] = 0.f;
	clearValue.color.float32[2] = 0.f;
	clearValue.color.float32[3] = 1.f;

	VkImageSubresourceRange range = {};
	range.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	range.baseMipLevel = 0;
	range.levelCount = 1;
	range.baseArrayLayer = 0;
	range.layerCount = 1;

	Vk::Image::TransitionImageLayoutDynamic(commandBuffer, frameBuffer->GetImage("Main")->Value(),
		VK_IMAGE_LAYOUT_UNDEFINED, VK_PIPELINE_STAGE_2_TOP_OF_PIPE_BIT, VK_ACCESS_2_NONE,
		VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_PIPELINE_STAGE_2_TRANSFER_BIT, VK_ACCESS_2_TRANSFER_WRITE_BIT);

	vkCmdClearColorImage(commandBuffer, frameBuffer->GetImage("Main")->Value(), VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, &clearValue.color, 1, &range);

	Vk::Image::TransitionImageLayoutDynamic(commandBuffer, frameBuffer->GetImage("Main")->Value(),
		VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_PIPELINE_STAGE_2_TRANSFER_BIT, VK_ACCESS_2_TRANSFER_WRITE_BIT,
		VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT, VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT);

	RenderDirectionLights(commandBuffer, registry, resourceManager, frameIndex);
	//RenderPointLights(commandBuffer, registry, resourceManager, frameIndex);
	//RenderSpotLights(commandBuffer, registry, resourceManager, frameIndex);

	RenderPointLightsIndirect(commandBuffer, registry, resourceManager, frameIndex);
	RenderSpotLightsIndirect(commandBuffer, registry, resourceManager, frameIndex);
}

void DeferredRenderer::RenderDirectionLights(VkCommandBuffer commandBuffer, std::shared_ptr<Registry> registry, std::shared_ptr<ResourceManager> resourceManager, uint32_t frameIndex)
{
	auto directionLightPool = registry->GetPool<DirectionLightComponent>();

	if (!directionLightPool || directionLightPool->GetDenseSize() == 0)
		return;

	auto vulkanContext = Vk::VulkanContext::GetContext();
	auto device = vulkanContext->GetDevice();
	auto graphicsQueue = device->GetQueue(Vk::QueueType::GRAPHICS);

	auto frameBuffer = resourceManager->GetVulkanManager()->GetFrameDependentFrameBuffer("Main", frameIndex);
	auto pipeline = resourceManager->GetVulkanManager()->GetGraphicsPipeline("DeferredDirectionLight");

	VkRenderingAttachmentInfo colorAttachment = Vk::DynamicRendering::BuildRenderingAttachmentInfo(frameBuffer->GetImage("Main")->GetImageView(), VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, nullptr);

	std::vector<VkRenderingAttachmentInfo> renderTargetAttachments = { colorAttachment };
	VkRenderingInfo renderingInfo = Vk::DynamicRendering::BuildRenderingInfo(frameBuffer->GetSize(), renderTargetAttachments, nullptr);

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

	DeferredDirectionLightPushConstants pushConstants;
	pushConstants.cameraIndex = 0;
	pushConstants.cameraBuffer = resourceManager->GetComponentBufferManager()->GetComponentBuffer("CameraData", frameIndex)->buffer->GetAddress();
	pushConstants.directionLightBufferAddress = resourceManager->GetComponentBufferManager()->GetComponentBuffer("DirectionLightData", frameIndex)->buffer->GetAddress();

	vkCmdPushConstants(commandBuffer, pipeline->GetLayout(), VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(DeferredDirectionLightPushConstants), &pushConstants);

	auto frameBufferDescriptorSet = resourceManager->GetVulkanManager()->GetFrameDependentDescriptorSet("MainFrameBuffer", frameIndex);
	vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline->GetLayout(), 0, 1, &frameBufferDescriptorSet->Value(), 0, nullptr);

	//TODO: BIND DIRLIGHT DYNAMIC DESCRIPTOR ARRAY INDICES

	vkCmdDraw(commandBuffer, 4, directionLightPool->GetDenseSize(), 0, 0);

	vkCmdEndRendering(commandBuffer);
}

void DeferredRenderer::RenderPointLights(VkCommandBuffer commandBuffer, std::shared_ptr<Registry> registry, std::shared_ptr<ResourceManager> resourceManager, uint32_t frameIndex)
{
	auto pointLightPool = registry->GetPool<PointLightComponent>();

	auto pointLightIndirectDrawBuffer = static_cast<VkDrawIndirectCommand*>(resourceManager->GetPointLightBufferManager()->GetIndirectDrawBuffer(frameIndex)->buffer->GetHandler());
	uint32_t pointLightInstanceCount = pointLightIndirectDrawBuffer[0].instanceCount;

	if (!pointLightPool || pointLightPool->GetDenseSize() == 0 || pointLightInstanceCount == 0)
		return;

	auto vulkanContext = Vk::VulkanContext::GetContext();
	auto device = vulkanContext->GetDevice();

	auto graphicsQueue = device->GetQueue(Vk::QueueType::GRAPHICS);

	auto frameBuffer = resourceManager->GetVulkanManager()->GetFrameDependentFrameBuffer("Main", frameIndex);
	auto pipeline = resourceManager->GetVulkanManager()->GetGraphicsPipeline("DeferredPointLight");

	VkRenderingAttachmentInfo colorAttachment = Vk::DynamicRendering::BuildRenderingAttachmentInfo(frameBuffer->GetImage("Main")->GetImageView(), VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, nullptr);
	VkRenderingAttachmentInfo depthAttachment = Vk::DynamicRendering::BuildRenderingAttachmentInfo(frameBuffer->GetImage("Depth")->GetImageView(), VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL, nullptr);
	
	std::vector<VkRenderingAttachmentInfo> renderTargetAttachments = { colorAttachment };
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

	DeferredPointLightPushConstants pushConstants;
	pushConstants.cameraIndex = 0; //TODO: MAIN CAMERA
	pushConstants.cameraBuffer = resourceManager->GetComponentBufferManager()->GetComponentBuffer("CameraData", frameIndex)->buffer->GetAddress();
	pushConstants.pointLightBufferAddress = resourceManager->GetComponentBufferManager()->GetComponentBuffer("PointLightData", frameIndex)->buffer->GetAddress();
	pushConstants.transformBufferAddress = resourceManager->GetComponentBufferManager()->GetComponentBuffer("PointLightTransform", frameIndex)->buffer->GetAddress();
	pushConstants.instanceBufferAddress = resourceManager->GetPointLightBufferManager()->GetInstanceIndexBuffer(frameIndex)->buffer->GetAddress();
	pushConstants.vertexBufferAddress = shape->GetVertexBuffer()->GetAddress();
	pushConstants.indexBufferAddress = shape->GetIndexBuffer()->GetAddress();
	pushConstants.viewPortSize = glm::vec2(viewport.width, viewport.height);

	vkCmdPushConstants(commandBuffer, pipeline->GetLayout(), VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(DeferredPointLightPushConstants), &pushConstants);

	auto frameBufferDescriptorSet = resourceManager->GetVulkanManager()->GetFrameDependentDescriptorSet("MainFrameBuffer", frameIndex);
	vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline->GetLayout(), 0, 1, &frameBufferDescriptorSet->Value(), 0, nullptr);

	//TODO: BIND POINT LIGHT DYNAMIC DESCRIPTOR ARRAY INDICES

	vkCmdDraw(commandBuffer, shape->GetIndexCount(), pointLightInstanceCount, 0, 0);

	vkCmdEndRendering(commandBuffer);
}

void DeferredRenderer::RenderSpotLights(VkCommandBuffer commandBuffer, std::shared_ptr<Registry> registry, std::shared_ptr<ResourceManager> resourceManager, uint32_t frameIndex)
{
	auto spotLightPool = registry->GetPool<SpotLightComponent>();

	auto spotLightIndirectDrawBuffer = static_cast<VkDrawIndirectCommand*>(resourceManager->GetSpotLightBufferManager()->GetIndirectDrawBuffer(frameIndex)->buffer->GetHandler());
	uint32_t spotLightInstanceCount = spotLightIndirectDrawBuffer[0].instanceCount;

	if (!spotLightPool || spotLightPool->GetDenseSize() == 0 || spotLightInstanceCount == 0)
		return;

	auto vulkanContext = Vk::VulkanContext::GetContext();
	auto device = vulkanContext->GetDevice();
	auto graphicsQueue = device->GetQueue(Vk::QueueType::GRAPHICS);

	auto frameBuffer = resourceManager->GetVulkanManager()->GetFrameDependentFrameBuffer("Main", frameIndex);
	auto pipeline = resourceManager->GetVulkanManager()->GetGraphicsPipeline("DeferredSpotLight");

	VkRenderingAttachmentInfo colorAttachment = Vk::DynamicRendering::BuildRenderingAttachmentInfo(frameBuffer->GetImage("Main")->GetImageView(), VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, nullptr);
	VkRenderingAttachmentInfo depthAttachment = Vk::DynamicRendering::BuildRenderingAttachmentInfo(frameBuffer->GetImage("Depth")->GetImageView(), VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL, nullptr);
	
	std::vector<VkRenderingAttachmentInfo> renderTargetAttachments = { colorAttachment };
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

	DeferredSpotLightPushConstants pushConstants;
	pushConstants.cameraIndex = 0; //TODO: MAIN CAMERA
	pushConstants.cameraBuffer = resourceManager->GetComponentBufferManager()->GetComponentBuffer("CameraData", frameIndex)->buffer->GetAddress();
	pushConstants.spotLightBufferAddress = resourceManager->GetComponentBufferManager()->GetComponentBuffer("SpotLightData", frameIndex)->buffer->GetAddress();
	pushConstants.transformBufferAddress = resourceManager->GetComponentBufferManager()->GetComponentBuffer("SpotLightTransform", frameIndex)->buffer->GetAddress();
	pushConstants.instanceBufferAddress = resourceManager->GetSpotLightBufferManager()->GetInstanceIndexBuffer(frameIndex)->buffer->GetAddress();
	pushConstants.vertexBufferAddress = shape->GetVertexBuffer()->GetAddress();
	pushConstants.indexBufferAddress = shape->GetIndexBuffer()->GetAddress();
	pushConstants.viewPortSize = glm::vec2(viewport.width, viewport.height);

	vkCmdPushConstants(commandBuffer, pipeline->GetLayout(), VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(DeferredSpotLightPushConstants), &pushConstants);

	auto frameBufferDescriptorSet = resourceManager->GetVulkanManager()->GetFrameDependentDescriptorSet("MainFrameBuffer", frameIndex);
	vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline->GetLayout(), 0, 1, &frameBufferDescriptorSet->Value(), 0, nullptr);

	//TODO: BIND Spot LIGHT DYNAMIC DESCRIPTOR ARRAY INDICES

	vkCmdDraw(commandBuffer, shape->GetIndexCount(), spotLightInstanceCount, 0, 0);

	vkCmdEndRendering(commandBuffer);
}

void DeferredRenderer::RenderPointLightsIndirect(VkCommandBuffer commandBuffer, std::shared_ptr<Registry> registry, std::shared_ptr<ResourceManager> resourceManager, uint32_t frameIndex)
{
	auto pointLightPool = registry->GetPool<PointLightComponent>();

	if (!pointLightPool || pointLightPool->GetDenseSize() == 0)
		return;

	auto vulkanContext = Vk::VulkanContext::GetContext();
	auto device = vulkanContext->GetDevice();

	auto graphicsQueue = device->GetQueue(Vk::QueueType::GRAPHICS);

	auto frameBuffer = resourceManager->GetVulkanManager()->GetFrameDependentFrameBuffer("Main", frameIndex);
	auto pipeline = resourceManager->GetVulkanManager()->GetGraphicsPipeline("DeferredPointLight");

	VkRenderingAttachmentInfo colorAttachment = Vk::DynamicRendering::BuildRenderingAttachmentInfo(frameBuffer->GetImage("Main")->GetImageView(), VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, nullptr);
	VkRenderingAttachmentInfo depthAttachment = Vk::DynamicRendering::BuildRenderingAttachmentInfo(frameBuffer->GetImage("Depth")->GetImageView(), VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL, nullptr);

	std::vector<VkRenderingAttachmentInfo> renderTargetAttachments = { colorAttachment };
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

	DeferredPointLightPushConstants pushConstants;
	pushConstants.cameraIndex = 0; //TODO: MAIN CAMERA
	pushConstants.cameraBuffer = resourceManager->GetComponentBufferManager()->GetComponentBuffer("CameraData", frameIndex)->buffer->GetAddress();
	pushConstants.pointLightBufferAddress = resourceManager->GetComponentBufferManager()->GetComponentBuffer("PointLightData", frameIndex)->buffer->GetAddress();
	pushConstants.transformBufferAddress = resourceManager->GetComponentBufferManager()->GetComponentBuffer("PointLightTransform", frameIndex)->buffer->GetAddress();
	pushConstants.instanceBufferAddress = resourceManager->GetPointLightBufferManager()->GetInstanceIndexBuffer(frameIndex)->buffer->GetAddress();
	pushConstants.vertexBufferAddress = shape->GetVertexBuffer()->GetAddress();
	pushConstants.indexBufferAddress = shape->GetIndexBuffer()->GetAddress();
	pushConstants.viewPortSize = glm::vec2(viewport.width, viewport.height);

	vkCmdPushConstants(commandBuffer, pipeline->GetLayout(), VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(DeferredPointLightPushConstants), &pushConstants);

	auto frameBufferDescriptorSet = resourceManager->GetVulkanManager()->GetFrameDependentDescriptorSet("MainFrameBuffer", frameIndex);
	vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline->GetLayout(), 0, 1, &frameBufferDescriptorSet->Value(), 0, nullptr);

	//TODO: BIND POINT LIGHT DYNAMIC DESCRIPTOR ARRAY INDICES

	vkCmdDrawIndirect(commandBuffer, resourceManager->GetPointLightBufferManager()->GetIndirectDrawBuffer(frameIndex)->buffer->Value(), 0, 1, sizeof(VkDrawIndirectCommand));

	vkCmdEndRendering(commandBuffer);
}

void DeferredRenderer::RenderSpotLightsIndirect(VkCommandBuffer commandBuffer, std::shared_ptr<Registry> registry, std::shared_ptr<ResourceManager> resourceManager, uint32_t frameIndex)
{
	auto spotLightPool = registry->GetPool<SpotLightComponent>();

	if (!spotLightPool || spotLightPool->GetDenseSize() == 0)
		return;

	auto vulkanContext = Vk::VulkanContext::GetContext();
	auto device = vulkanContext->GetDevice();

	auto graphicsQueue = device->GetQueue(Vk::QueueType::GRAPHICS);

	auto frameBuffer = resourceManager->GetVulkanManager()->GetFrameDependentFrameBuffer("Main", frameIndex);
	auto pipeline = resourceManager->GetVulkanManager()->GetGraphicsPipeline("DeferredSpotLight");

	VkRenderingAttachmentInfo colorAttachment = Vk::DynamicRendering::BuildRenderingAttachmentInfo(frameBuffer->GetImage("Main")->GetImageView(), VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, nullptr);
	VkRenderingAttachmentInfo depthAttachment = Vk::DynamicRendering::BuildRenderingAttachmentInfo(frameBuffer->GetImage("Depth")->GetImageView(), VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL, nullptr);

	std::vector<VkRenderingAttachmentInfo> renderTargetAttachments = { colorAttachment };
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

	DeferredSpotLightPushConstants pushConstants;
	pushConstants.cameraIndex = 0; //TODO: MAIN CAMERA
	pushConstants.cameraBuffer = resourceManager->GetComponentBufferManager()->GetComponentBuffer("CameraData", frameIndex)->buffer->GetAddress();
	pushConstants.spotLightBufferAddress = resourceManager->GetComponentBufferManager()->GetComponentBuffer("SpotLightData", frameIndex)->buffer->GetAddress();
	pushConstants.transformBufferAddress = resourceManager->GetComponentBufferManager()->GetComponentBuffer("SpotLightTransform", frameIndex)->buffer->GetAddress();
	pushConstants.instanceBufferAddress = resourceManager->GetSpotLightBufferManager()->GetInstanceIndexBuffer(frameIndex)->buffer->GetAddress();
	pushConstants.vertexBufferAddress = shape->GetVertexBuffer()->GetAddress();
	pushConstants.indexBufferAddress = shape->GetIndexBuffer()->GetAddress();
	pushConstants.viewPortSize = glm::vec2(viewport.width, viewport.height);

	vkCmdPushConstants(commandBuffer, pipeline->GetLayout(), VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(DeferredSpotLightPushConstants), &pushConstants);

	auto frameBufferDescriptorSet = resourceManager->GetVulkanManager()->GetFrameDependentDescriptorSet("MainFrameBuffer", frameIndex);
	vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline->GetLayout(), 0, 1, &frameBufferDescriptorSet->Value(), 0, nullptr);

	//TODO: BIND POINT LIGHT DYNAMIC DESCRIPTOR ARRAY INDICES

	vkCmdDrawIndirect(commandBuffer, resourceManager->GetSpotLightBufferManager()->GetIndirectDrawBuffer(frameIndex)->buffer->Value(), 0, 1, sizeof(VkDrawIndirectCommand));

	vkCmdEndRendering(commandBuffer);
}