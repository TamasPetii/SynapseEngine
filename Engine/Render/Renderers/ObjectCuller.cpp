#include "ObjectCuller.h"
#include "Engine/Render/GpuStructs.h"
#include "Engine/Components/DefaultColliderComponent.h"
#include "Engine/Components/PointLightComponent.h"

void ObjectCuller::Initialize(std::shared_ptr<ResourceManager> resourceManager)
{
}

void ObjectCuller::Render(VkCommandBuffer commandBuffer, std::shared_ptr<Registry> registry, std::shared_ptr<ResourceManager> resourceManager, uint32_t frameIndex, std::function<void()> renderFunction)
{
	CullObjectInCameraFrustum(commandBuffer, registry, resourceManager, frameIndex);
	CullPointLightInCameraFrustum(commandBuffer, registry, resourceManager, frameIndex);
}

void ObjectCuller::CullObjectInCameraFrustum(VkCommandBuffer commandBuffer, std::shared_ptr<Registry> registry, std::shared_ptr<ResourceManager> resourceManager, uint32_t frameIndex)
{
	auto vulkanContext = Vk::VulkanContext::GetContext();
	auto device = vulkanContext->GetDevice();
	auto graphicsQueue = device->GetQueue(Vk::QueueType::GRAPHICS);
	auto pipeline = resourceManager->GetVulkanManager()->GetComputePipeline("CullingCameraFrustum");

	auto modelIndirectDrawBufferHandler = static_cast<VkDrawIndirectCommand*>(resourceManager->GetModelManager()->GetIndirectDrawBuffer(frameIndex)->buffer->GetHandler());
	auto shapeIndirectDrawBufferHandler = static_cast<VkDrawIndirectCommand*>(resourceManager->GetGeometryManager()->GetIndirectDrawBuffer(frameIndex)->buffer->GetHandler());

	//Todo: Handle model reposition, iterate over models + Problems with async? IndirectCountBuffer
	for (uint32_t i = 0; i < resourceManager->GetModelManager()->GetCurrentCount(); ++i)
		modelIndirectDrawBufferHandler[i].instanceCount = 0;

	for (uint32_t i = 0; i < resourceManager->GetGeometryManager()->GetCurrentCount(); ++i)
		shapeIndirectDrawBufferHandler[i].instanceCount = 0;

	uint32_t defaultColliderCount = registry->GetPool<DefaultColliderComponent>()->GetDenseSize();

	uint32_t workgroupSize = 64; // Matches shader local_size_x
	uint32_t workgroupCount = static_cast<uint32_t>(std::ceil(defaultColliderCount / (float)workgroupSize));

	if (workgroupCount == 0)
		return;

	CullingCameraFrustumPushConstants pushConstants;
	pushConstants.cameraIndex = 0;
	pushConstants.cameraFrustumBuffer = resourceManager->GetComponentBufferManager()->GetComponentBuffer("CameraFrustumData", frameIndex)->buffer->GetAddress();

	pushConstants.defaultColliderCount = defaultColliderCount;
	pushConstants.defaultColliderBuffer = resourceManager->GetComponentBufferManager()->GetComponentBuffer("DefaultColliderData", frameIndex)->buffer->GetAddress();

	pushConstants.modelRenderIndicesBuffer = resourceManager->GetComponentBufferManager()->GetComponentBuffer("ModelRenderIndicesData", frameIndex)->buffer->GetAddress();
	pushConstants.modelBufferAddresses = resourceManager->GetModelManager()->GetDeviceAddressesBuffer(frameIndex)->buffer->GetAddress();
	pushConstants.modelDrawIndirectCommandBuffer = resourceManager->GetModelManager()->GetIndirectDrawBuffer(frameIndex)->buffer->GetAddress();
	pushConstants.modelInstanceIndexAddressBuffer = resourceManager->GetModelManager()->GetInstanceIndexAddressBuffer(frameIndex)->buffer->GetAddress();

	pushConstants.shapeRenderIndicesBuffer = resourceManager->GetComponentBufferManager()->GetComponentBuffer("ShapeRenderIndicesData", frameIndex)->buffer->GetAddress();
	pushConstants.shapeBufferAddresses = resourceManager->GetGeometryManager()->GetDeviceAddressesBuffer(frameIndex)->buffer->GetAddress();
	pushConstants.shapeDrawIndirectCommandBuffer = resourceManager->GetGeometryManager()->GetIndirectDrawBuffer(frameIndex)->buffer->GetAddress();
	pushConstants.shapeInstanceIndexAddressBuffer = resourceManager->GetGeometryManager()->GetInstanceIndexAddressBuffer(frameIndex)->buffer->GetAddress();

	vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipeline->GetPipeline());
	vkCmdPushConstants(commandBuffer, pipeline->GetLayout(), VK_SHADER_STAGE_COMPUTE_BIT, 0, sizeof(CullingCameraFrustumPushConstants), &pushConstants);

	vkCmdDispatch(commandBuffer, workgroupCount, 1, 1);

	//Todo!
	VkMemoryBarrier2 drawBarrier = {};
	drawBarrier.sType = VK_STRUCTURE_TYPE_MEMORY_BARRIER_2;
	drawBarrier.srcStageMask = VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT;
	drawBarrier.srcAccessMask = VK_ACCESS_2_SHADER_WRITE_BIT;
	drawBarrier.dstStageMask = VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT;
	drawBarrier.dstAccessMask = VK_ACCESS_2_SHADER_READ_BIT | VK_ACCESS_2_SHADER_WRITE_BIT;

	VkDependencyInfo drawDepInfo = {};
	drawDepInfo.sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO;
	drawDepInfo.memoryBarrierCount = 1;
	drawDepInfo.pMemoryBarriers = &drawBarrier;

	vkCmdPipelineBarrier2(commandBuffer, &drawDepInfo);
}

void ObjectCuller::CullPointLightInCameraFrustum(VkCommandBuffer commandBuffer, std::shared_ptr<Registry> registry, std::shared_ptr<ResourceManager> resourceManager, uint32_t frameIndex)
{
	auto vulkanContext = Vk::VulkanContext::GetContext();
	auto device = vulkanContext->GetDevice();
	auto graphicsQueue = device->GetQueue(Vk::QueueType::GRAPHICS);
	auto pipeline = resourceManager->GetVulkanManager()->GetComputePipeline("CullingPointLight");

	auto pointLightShadowCountBufferHandler = static_cast<uint32_t*>(resourceManager->GetPointLightBufferManager()->GetShadowCountBuffer(frameIndex)->buffer->GetHandler());
	pointLightShadowCountBufferHandler[0] = 0;
	
	auto pointLightShadowDispatchIndirectBuffers = static_cast<VkDispatchIndirectCommand*>(resourceManager->GetPointLightBufferManager()->GetShadowDispatchIndirectBuffers(frameIndex)->buffer->GetHandler());
	pointLightShadowDispatchIndirectBuffers[0] = VkDispatchIndirectCommand{
		.x = 1,
		.y = 1,
		.z = 1
	};
	
	auto pointLightIndirectDrawBufferHandler = static_cast<VkDrawIndirectCommand*>(resourceManager->GetPointLightBufferManager()->GetIndirectDrawBuffer(frameIndex)->buffer->GetHandler());
	pointLightIndirectDrawBufferHandler[0] = VkDrawIndirectCommand{
		.vertexCount = resourceManager->GetGeometryManager()->GetShape("Cube")->GetIndexCount(),
		.instanceCount = 0,
		.firstVertex = 0,
		.firstInstance = 0
	};

	uint32_t pointLightCount = registry->GetPool<PointLightComponent>()->GetDenseSize();

	uint32_t workgroupSize = 64; // Matches shader local_size_x
	uint32_t workgroupCount = static_cast<uint32_t>(std::ceil(pointLightCount / (float)workgroupSize));

	if (workgroupCount == 0)
		return;

	VkDeviceAddress pointLightShadowDispatchIndirectBuffer;

	CullingPointLightPushConstants pushConstants;
	pushConstants.cameraIndex = 0;
	pushConstants.cameraFrustumBuffer = resourceManager->GetComponentBufferManager()->GetComponentBuffer("CameraFrustumData", frameIndex)->buffer->GetAddress();
	pushConstants.pointLightCount = pointLightCount;
	pushConstants.pointLightBuffer = resourceManager->GetComponentBufferManager()->GetComponentBuffer("PointLightData", frameIndex)->buffer->GetAddress();
	pushConstants.pointLightInstanceIndexBuffer = resourceManager->GetPointLightBufferManager()->GetInstanceIndexBuffer(frameIndex)->buffer->GetAddress();
	pushConstants.pointLightDrawIndirectCommandBuffer = resourceManager->GetPointLightBufferManager()->GetIndirectDrawBuffer(frameIndex)->buffer->GetAddress();
	pushConstants.pointLightShadowInstanceIndexBuffer = resourceManager->GetPointLightBufferManager()->GetShadowInstanceIndexBuffer(frameIndex)->buffer->GetAddress();
	pushConstants.pointLightShadowCountBuffer = resourceManager->GetPointLightBufferManager()->GetShadowCountBuffer(frameIndex)->buffer->GetAddress();
	pushConstants.pointLightShadowDispatchIndirectBuffer = resourceManager->GetPointLightBufferManager()->GetShadowDispatchIndirectBuffers(frameIndex)->buffer->GetAddress();

	vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipeline->GetPipeline());
	vkCmdPushConstants(commandBuffer, pipeline->GetLayout(), VK_SHADER_STAGE_COMPUTE_BIT, 0, sizeof(CullingPointLightPushConstants), &pushConstants);

	vkCmdDispatch(commandBuffer, workgroupCount, 1, 1);

	VkMemoryBarrier2 drawBarrier = {};
	drawBarrier.sType = VK_STRUCTURE_TYPE_MEMORY_BARRIER_2;
	drawBarrier.srcStageMask = VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT;
	drawBarrier.srcAccessMask = VK_ACCESS_2_SHADER_WRITE_BIT;
	drawBarrier.dstStageMask = VK_PIPELINE_STAGE_2_DRAW_INDIRECT_BIT | VK_PIPELINE_STAGE_2_VERTEX_SHADER_BIT;
	drawBarrier.dstAccessMask = VK_ACCESS_2_INDIRECT_COMMAND_READ_BIT | VK_ACCESS_2_SHADER_READ_BIT;

	VkDependencyInfo drawDepInfo = {};
	drawDepInfo.sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO;
	drawDepInfo.memoryBarrierCount = 1;
	drawDepInfo.pMemoryBarriers = &drawBarrier;

	vkCmdPipelineBarrier2(commandBuffer, &drawDepInfo);
}