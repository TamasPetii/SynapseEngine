#include "ObjectCuller.h"
#include "Engine/Render/GpuStructs.h"
#include "Engine/Components/DefaultColliderComponent.h"
#include "Engine/Components/PointLightComponent.h"
#include "Engine/Components/SpotLightComponent.h"

void ObjectCuller::Initialize(std::shared_ptr<ResourceManager> resourceManager)
{
}

void ObjectCuller::Render(VkCommandBuffer commandBuffer, std::shared_ptr<Registry> registry, std::shared_ptr<ResourceManager> resourceManager, uint32_t frameIndex, std::function<void()> renderFunction)
{
	CullPointLightInCameraFrustum(commandBuffer, registry, resourceManager, frameIndex);
	CullSpotLightInCameraFrustum(commandBuffer, registry, resourceManager, frameIndex);
	CullObjectInCameraFrustum(commandBuffer, registry, resourceManager, frameIndex);
	CullPointLightShadowObjects(commandBuffer, registry, resourceManager, frameIndex);
	CullSpotLightShadowObjects(commandBuffer, registry, resourceManager, frameIndex);
}

void ObjectCuller::CullObjectInCameraFrustum(VkCommandBuffer commandBuffer, std::shared_ptr<Registry> registry, std::shared_ptr<ResourceManager> resourceManager, uint32_t frameIndex)
{
	auto vulkanContext = Vk::VulkanContext::GetContext();
	auto device = vulkanContext->GetDevice();
	auto graphicsQueue = device->GetQueue(Vk::QueueType::GRAPHICS);
	auto pipeline = resourceManager->GetVulkanManager()->GetComputePipeline("CullingCameraFrustum");
	auto pushDescriptor = resourceManager->GetVulkanManager()->GetPushDescriptorSet("DepthPyramid");

	//Fix: Problematic with framebuffer resizing?
	auto previousFrameIndex = (frameIndex + GlobalConfig::FrameConfig::framesInFlight - 1) % GlobalConfig::FrameConfig::framesInFlight;
	auto previousFrameBuffer = resourceManager->GetVulkanManager()->GetFrameDependentFrameBuffer("Main", previousFrameIndex);

	auto modelIndirectDrawBufferHandler = static_cast<VkDrawIndirectCommand*>(resourceManager->GetModelManager()->GetIndirectDrawBuffer(frameIndex)->buffer->GetHandler());
	auto shapeIndirectDrawBufferHandler = static_cast<VkDrawIndirectCommand*>(resourceManager->GetGeometryManager()->GetIndirectDrawBuffer(frameIndex)->buffer->GetHandler());

	//Todo: Handle model reposition, iterate over models + Problems with async? IndirectCountBuffer
	for (uint32_t i = 0; i < resourceManager->GetModelManager()->GetCurrentCount(); ++i)
		modelIndirectDrawBufferHandler[i].instanceCount = 0;

	for (uint32_t i = 0; i < resourceManager->GetGeometryManager()->GetCurrentCount(); ++i)
		shapeIndirectDrawBufferHandler[i].instanceCount = 0;

	if (!registry->GetPool<DefaultColliderComponent>())
		return;

	uint32_t defaultColliderCount = registry->GetPool<DefaultColliderComponent>()->GetDenseSize();

	uint32_t workgroupSize = 64; // Matches shader local_size_x
	uint32_t workgroupCount = static_cast<uint32_t>(std::ceil(defaultColliderCount / (float)workgroupSize));

	if (workgroupCount == 0)
		return;

	CullingCameraFrustumPushConstants pushConstants;
	pushConstants.cameraIndex = 0;
	pushConstants.cameraBuffer = resourceManager->GetComponentBufferManager()->GetComponentBuffer("CameraData", frameIndex)->buffer->GetAddress();
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

	pushConstants.depthPyramidSize = glm::vec2(previousFrameBuffer->GetSize().width, previousFrameBuffer->GetSize().height);

	vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipeline->GetPipeline());
	vkCmdPushConstants(commandBuffer, pipeline->GetLayout(), VK_SHADER_STAGE_COMPUTE_BIT, 0, sizeof(CullingCameraFrustumPushConstants), &pushConstants);
	
	VkDescriptorImageInfo srcTarget;
	srcTarget.sampler = resourceManager->GetVulkanManager()->GetSampler("MaxReduction")->Value();
	srcTarget.imageView = previousFrameBuffer->GetImage("DepthPyramid")->GetImageView("Default");
	srcTarget.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

	//Potential problem with srcTarget in BuildWritePushDescriptorSetInfo image info???
	std::vector<VkWriteDescriptorSet> descriptorWrites =
	{
		Vk::DesciptorSetUtils::BuildWritePushDescriptorSetInfo(pushDescriptor->GetImageLayout("SrcImage"), srcTarget)
	};

	device->vkCmdPushDescriptorSetKHR(commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipeline->GetLayout(), 0, descriptorWrites.size(), descriptorWrites.data());

	vkCmdDispatch(commandBuffer, workgroupCount, 1, 1);

	//Todo!
	/*
	VkMemoryBarrier2 drawBarrier = {};
	drawBarrier.sType = VK_STRUCTURE_TYPE_MEMORY_BARRIER_2;
	drawBarrier.srcStageMask = VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT;
	drawBarrier.srcAccessMask = VK_ACCESS_2_SHADER_WRITE_BIT;
	drawBarrier.dstStageMask = VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT;
	drawBarrier.dstAccessMask = VK_ACCESS_2_SHADER_READ_BIT | VK_ACCESS_2_SHADER_WRITE_BIT;
	*/

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

	/*
	
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
	*/
}

void ObjectCuller::CullPointLightInCameraFrustum(VkCommandBuffer commandBuffer, std::shared_ptr<Registry> registry, std::shared_ptr<ResourceManager> resourceManager, uint32_t frameIndex)
{
	auto vulkanContext = Vk::VulkanContext::GetContext();
	auto device = vulkanContext->GetDevice();
	auto graphicsQueue = device->GetQueue(Vk::QueueType::GRAPHICS);
	auto pipeline = resourceManager->GetVulkanManager()->GetComputePipeline("CullingPointLight");
	auto pushDescriptor = resourceManager->GetVulkanManager()->GetPushDescriptorSet("DepthPyramid");
	auto previousFrameIndex = (frameIndex + GlobalConfig::FrameConfig::framesInFlight - 1) % GlobalConfig::FrameConfig::framesInFlight;
	auto previousFrameBuffer = resourceManager->GetVulkanManager()->GetFrameDependentFrameBuffer("Main", previousFrameIndex);

	auto pointLightShadowCountBufferHandler = static_cast<LightBufferCommonData*>(resourceManager->GetPointLightBufferManager()->GetCommonDataBuffer(frameIndex)->buffer->GetHandler());
	pointLightShadowCountBufferHandler[0] = LightBufferCommonData{};
	
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

	if (!registry->GetPool<PointLightComponent>())
		return;

	uint32_t pointLightCount = registry->GetPool<PointLightComponent>()->GetDenseSize();

	uint32_t workgroupSize = 64; // Matches shader local_size_x
	uint32_t workgroupCount = static_cast<uint32_t>(std::ceil(pointLightCount / (float)workgroupSize));

	if (workgroupCount == 0)
		return;

	CullingPointLightPushConstants pushConstants;
	pushConstants.cameraIndex = 0;
	pushConstants.cameraBuffer = resourceManager->GetComponentBufferManager()->GetComponentBuffer("CameraData", frameIndex)->buffer->GetAddress();
	pushConstants.cameraFrustumBuffer = resourceManager->GetComponentBufferManager()->GetComponentBuffer("CameraFrustumData", frameIndex)->buffer->GetAddress();
	pushConstants.pointLightCount = pointLightCount;
	pushConstants.pointLightBuffer = resourceManager->GetComponentBufferManager()->GetComponentBuffer("PointLightData", frameIndex)->buffer->GetAddress();
	pushConstants.pointLightInstanceIndexBuffer = resourceManager->GetPointLightBufferManager()->GetInstanceIndexBuffer(frameIndex)->buffer->GetAddress();
	pushConstants.pointLightDrawIndirectCommandBuffer = resourceManager->GetPointLightBufferManager()->GetIndirectDrawBuffer(frameIndex)->buffer->GetAddress();
	pushConstants.pointLightShadowInstanceIndexBuffer = resourceManager->GetPointLightBufferManager()->GetShadowInstanceIndexBuffer(frameIndex)->buffer->GetAddress();
	pushConstants.pointLightCommonDataBuffer = resourceManager->GetPointLightBufferManager()->GetCommonDataBuffer(frameIndex)->buffer->GetAddress();
	pushConstants.pointLightShadowDispatchIndirectBuffer = resourceManager->GetPointLightBufferManager()->GetShadowDispatchIndirectBuffers(frameIndex)->buffer->GetAddress();
	pushConstants.colliderDebugBuffer = resourceManager->GetComponentBufferManager()->GetComponentBuffer("PointLightColliderDebug", frameIndex)->buffer->GetAddress();
	pushConstants.depthPyramidSize = glm::vec2(previousFrameBuffer->GetSize().width, previousFrameBuffer->GetSize().height);

	vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipeline->GetPipeline());
	vkCmdPushConstants(commandBuffer, pipeline->GetLayout(), VK_SHADER_STAGE_COMPUTE_BIT, 0, sizeof(CullingPointLightPushConstants), &pushConstants);

	vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipeline->GetPipeline());
	vkCmdPushConstants(commandBuffer, pipeline->GetLayout(), VK_SHADER_STAGE_COMPUTE_BIT, 0, sizeof(CullingSpotLightPushConstants), &pushConstants);

	VkDescriptorImageInfo srcTarget;
	srcTarget.sampler = resourceManager->GetVulkanManager()->GetSampler("MaxReduction")->Value();
	srcTarget.imageView = previousFrameBuffer->GetImage("DepthPyramid")->GetImageView("Default");
	srcTarget.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

	//Potential problem with srcTarget in BuildWritePushDescriptorSetInfo image info???
	std::vector<VkWriteDescriptorSet> descriptorWrites =
	{
		Vk::DesciptorSetUtils::BuildWritePushDescriptorSetInfo(pushDescriptor->GetImageLayout("SrcImage"), srcTarget)
	};

	device->vkCmdPushDescriptorSetKHR(commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipeline->GetLayout(), 0, descriptorWrites.size(), descriptorWrites.data());

	vkCmdDispatch(commandBuffer, workgroupCount, 1, 1);
}

void ObjectCuller::CullPointLightShadowObjects(VkCommandBuffer commandBuffer, std::shared_ptr<Registry> registry, std::shared_ptr<ResourceManager> resourceManager, uint32_t frameIndex)
{
}

void ObjectCuller::CullSpotLightInCameraFrustum(VkCommandBuffer commandBuffer, std::shared_ptr<Registry> registry, std::shared_ptr<ResourceManager> resourceManager, uint32_t frameIndex)
{
	auto vulkanContext = Vk::VulkanContext::GetContext();
	auto device = vulkanContext->GetDevice();
	auto graphicsQueue = device->GetQueue(Vk::QueueType::GRAPHICS);
	auto pipeline = resourceManager->GetVulkanManager()->GetComputePipeline("CullingSpotLight");
	auto pushDescriptor = resourceManager->GetVulkanManager()->GetPushDescriptorSet("DepthPyramid");
	auto previousFrameIndex = (frameIndex + GlobalConfig::FrameConfig::framesInFlight - 1) % GlobalConfig::FrameConfig::framesInFlight;
	auto previousFrameBuffer = resourceManager->GetVulkanManager()->GetFrameDependentFrameBuffer("Main", previousFrameIndex);

	auto spotLightShadowCountBufferHandler = static_cast<LightBufferCommonData*>(resourceManager->GetSpotLightBufferManager()->GetCommonDataBuffer(frameIndex)->buffer->GetHandler());
	spotLightShadowCountBufferHandler[0] = LightBufferCommonData{};;

	auto spotLightShadowDispatchIndirectBuffers = static_cast<VkDispatchIndirectCommand*>(resourceManager->GetSpotLightBufferManager()->GetShadowDispatchIndirectBuffers(frameIndex)->buffer->GetHandler());
	spotLightShadowDispatchIndirectBuffers[0] = VkDispatchIndirectCommand{
		.x = 1,
		.y = 1,
		.z = 1
	};

	auto spotLightIndirectDrawBufferHandler = static_cast<VkDrawIndirectCommand*>(resourceManager->GetSpotLightBufferManager()->GetIndirectDrawBuffer(frameIndex)->buffer->GetHandler());
	spotLightIndirectDrawBufferHandler[0] = VkDrawIndirectCommand{
		.vertexCount = resourceManager->GetGeometryManager()->GetShape("Cone")->GetIndexCount(),
		.instanceCount = 0,
		.firstVertex = 0,
		.firstInstance = 0
	};

	if (!registry->GetPool<SpotLightComponent>())
		return;

	uint32_t spotLightCount = registry->GetPool<SpotLightComponent>()->GetDenseSize();

	uint32_t workgroupSize = 64; // Matches shader local_size_x
	uint32_t workgroupCount = static_cast<uint32_t>(std::ceil(spotLightCount / (float)workgroupSize));

	if (workgroupCount == 0)
		return;

	CullingSpotLightPushConstants pushConstants;
	pushConstants.cameraIndex = 0;
	pushConstants.cameraBuffer = resourceManager->GetComponentBufferManager()->GetComponentBuffer("CameraData", frameIndex)->buffer->GetAddress();
	pushConstants.cameraFrustumBuffer = resourceManager->GetComponentBufferManager()->GetComponentBuffer("CameraFrustumData", frameIndex)->buffer->GetAddress();
	pushConstants.spotLightCount = spotLightCount;
	pushConstants.spotLightBuffer = resourceManager->GetComponentBufferManager()->GetComponentBuffer("SpotLightData", frameIndex)->buffer->GetAddress();
	pushConstants.spotLightInstanceIndexBuffer = resourceManager->GetSpotLightBufferManager()->GetInstanceIndexBuffer(frameIndex)->buffer->GetAddress();
	pushConstants.spotLightDrawIndirectCommandBuffer = resourceManager->GetSpotLightBufferManager()->GetIndirectDrawBuffer(frameIndex)->buffer->GetAddress();
	pushConstants.spotLightShadowInstanceIndexBuffer = resourceManager->GetSpotLightBufferManager()->GetShadowInstanceIndexBuffer(frameIndex)->buffer->GetAddress();
	pushConstants.spotLightCommonDataBuffer = resourceManager->GetSpotLightBufferManager()->GetCommonDataBuffer(frameIndex)->buffer->GetAddress();
	pushConstants.spotLightShadowDispatchIndirectBuffer = resourceManager->GetSpotLightBufferManager()->GetShadowDispatchIndirectBuffers(frameIndex)->buffer->GetAddress();
	pushConstants.colliderDebugBuffer = resourceManager->GetComponentBufferManager()->GetComponentBuffer("SpotLightColliderDebug", frameIndex)->buffer->GetAddress();
	pushConstants.depthPyramidSize = glm::vec2(previousFrameBuffer->GetSize().width, previousFrameBuffer->GetSize().height);

	vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipeline->GetPipeline());
	vkCmdPushConstants(commandBuffer, pipeline->GetLayout(), VK_SHADER_STAGE_COMPUTE_BIT, 0, sizeof(CullingSpotLightPushConstants), &pushConstants);

	VkDescriptorImageInfo srcTarget;
	srcTarget.sampler = resourceManager->GetVulkanManager()->GetSampler("MaxReduction")->Value();
	srcTarget.imageView = previousFrameBuffer->GetImage("DepthPyramid")->GetImageView("Default");
	srcTarget.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

	//Potential problem with srcTarget in BuildWritePushDescriptorSetInfo image info???
	std::vector<VkWriteDescriptorSet> descriptorWrites =
	{
		Vk::DesciptorSetUtils::BuildWritePushDescriptorSetInfo(pushDescriptor->GetImageLayout("SrcImage"), srcTarget)
	};

	device->vkCmdPushDescriptorSetKHR(commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipeline->GetLayout(), 0, descriptorWrites.size(), descriptorWrites.data());

	vkCmdDispatch(commandBuffer, workgroupCount, 1, 1);
}

void ObjectCuller::CullSpotLightShadowObjects(VkCommandBuffer commandBuffer, std::shared_ptr<Registry> registry, std::shared_ptr<ResourceManager> resourceManager, uint32_t frameIndex)
{
}

/*
void ObjectCuller::CullPointLightShadowObjects(VkCommandBuffer commandBuffer, std::shared_ptr<Registry> registry, std::shared_ptr<ResourceManager> resourceManager, uint32_t frameIndex)
{
	auto vulkanContext = Vk::VulkanContext::GetContext();
	auto device = vulkanContext->GetDevice();
	auto pointLightBufferManager = resourceManager->GetPointLightBufferManager();

	// 1. Clear Shadow Object Counters
	// Reset the counter for the "Pre-Pass" (list of colliders inside common light AABB)
	auto colliderCountBufferHandler = static_cast<uint32_t*>(pointLightBufferManager->GetShadowColliderCountBuffer(frameIndex)->buffer->GetHandler());
	colliderCountBufferHandler[0] = 0;

	// Reset the Indirect Draw Instance Counts for all shadow maps
	// Note: We zero out the memory because the shader uses atomicAdd
	auto modelIndirectDrawBuffer = pointLightBufferManager->GetShadowModelIndirectDrawBuffer(frameIndex)->buffer;
	auto shapeIndirectDrawBuffer = pointLightBufferManager->GetShadowShapeIndirectDrawBuffer(frameIndex)->buffer;

	vkCmdFillBuffer(commandBuffer, modelIndirectDrawBuffer->GetBuffer(), 0, modelIndirectDrawBuffer->GetSize(), 0);
	vkCmdFillBuffer(commandBuffer, shapeIndirectDrawBuffer->GetBuffer(), 0, shapeIndirectDrawBuffer->GetSize(), 0);

	// Ensure fills are visible to compute
	VkMemoryBarrier2 fillBarrier = { VK_STRUCTURE_TYPE_MEMORY_BARRIER_2 };
	fillBarrier.srcStageMask = VK_PIPELINE_STAGE_2_TRANSFER_BIT;
	fillBarrier.srcAccessMask = VK_ACCESS_2_TRANSFER_WRITE_BIT;
	fillBarrier.dstStageMask = VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT;
	fillBarrier.dstAccessMask = VK_ACCESS_2_SHADER_READ_BIT | VK_ACCESS_2_SHADER_WRITE_BIT;

	VkDependencyInfo fillDepInfo = { VK_STRUCTURE_TYPE_DEPENDENCY_INFO };
	fillDepInfo.memoryBarrierCount = 1;
	fillDepInfo.pMemoryBarriers = &fillBarrier;
	vkCmdPipelineBarrier2(commandBuffer, &fillDepInfo);

	// 2. Pass 1: Global AABB Culling (Reduce workload)
	// Checks all colliders against the union of all Point Light Shadow AABBs
	{
		if (!registry->GetPool<DefaultColliderComponent>()) return;
		uint32_t defaultColliderCount = registry->GetPool<DefaultColliderComponent>()->GetDenseSize();
		uint32_t workgroupSize = 64;
		uint32_t workgroupCount = static_cast<uint32_t>(std::ceil(defaultColliderCount / (float)workgroupSize));

		auto pipeline = resourceManager->GetVulkanManager()->GetComputePipeline("CullingLightShadowAabb");

		CullingLightShadowAabbPushConstants pushConstants;
		pushConstants.defaultColliderBuffer = resourceManager->GetComponentBufferManager()->GetComponentBuffer("DefaultColliderData", frameIndex)->buffer->GetAddress();
		pushConstants.defaultColliderCountBuffer = pointLightBufferManager->GetShadowColliderCountBuffer(frameIndex)->buffer->GetAddress(); [cite_start]// [cite: 14]
			pushConstants.defaultColliderInstanceIndexBuffer = pointLightBufferManager->GetShadowColliderIndexBuffer(frameIndex)->buffer->GetAddress(); [cite_start]// [cite: 18]
			pushConstants.lightCommonDataBuffer = pointLightBufferManager->GetCommonDataBuffer(frameIndex)->buffer->GetAddress();
		pushConstants.lightShadowDispatchIndirectBuffer = pointLightBufferManager->GetShadowDispatchIndirectBuffers(frameIndex)->buffer->GetAddress(); [cite_start]// [cite: 19]
			pushConstants.defaultColliderCount = defaultColliderCount;

		vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipeline->GetPipeline());
		vkCmdPushConstants(commandBuffer, pipeline->GetLayout(), VK_SHADER_STAGE_COMPUTE_BIT, 0, sizeof(CullingLightShadowAabbPushConstants), &pushConstants);
		vkCmdDispatch(commandBuffer, workgroupCount, 1, 1);
	}

	// Barrier: Wait for Pass 1 to update the Collider Index List and the Dispatch Indirect Buffer
	VkMemoryBarrier2 dispatchBarrier = { VK_STRUCTURE_TYPE_MEMORY_BARRIER_2 };
	dispatchBarrier.srcStageMask = VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT;
	dispatchBarrier.srcAccessMask = VK_ACCESS_2_SHADER_WRITE_BIT;
	dispatchBarrier.dstStageMask = VK_PIPELINE_STAGE_2_DRAW_INDIRECT_BIT | VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT;
	dispatchBarrier.dstAccessMask = VK_ACCESS_2_INDIRECT_COMMAND_READ_BIT | VK_ACCESS_2_SHADER_READ_BIT;

	VkDependencyInfo dispatchDepInfo = { VK_STRUCTURE_TYPE_DEPENDENCY_INFO };
	dispatchDepInfo.memoryBarrierCount = 1;
	dispatchDepInfo.pMemoryBarriers = &dispatchBarrier;
	vkCmdPipelineBarrier2(commandBuffer, &dispatchDepInfo);

	// 3. Pass 2: Exact Object vs Light Culling
	// Dispatched indirectly based on (ActiveLights x PotentialColliders)
	{
		auto pipeline = resourceManager->GetVulkanManager()->GetComputePipeline("CullingPointLightShadowObjects");

		CullingPointLightShadowObjectsPushConstants pushConstants;
		pushConstants.defaultColliderBuffer = resourceManager->GetComponentBufferManager()->GetComponentBuffer("DefaultColliderData", frameIndex)->buffer->GetAddress();
		pushConstants.defaultColliderCountBuffer = pointLightBufferManager->GetShadowColliderCountBuffer(frameIndex)->buffer->GetAddress();
		pushConstants.defaultColliderCulledIndexBuffer = pointLightBufferManager->GetShadowColliderIndexBuffer(frameIndex)->buffer->GetAddress();

		pushConstants.pointLightBuffer = resourceManager->GetComponentBufferManager()->GetComponentBuffer("PointLightData", frameIndex)->buffer->GetAddress();
		pushConstants.pointLightCommonDataBuffer = pointLightBufferManager->GetCommonDataBuffer(frameIndex)->buffer->GetAddress();
		pushConstants.pointLightShadowInstanceIndexBuffer = pointLightBufferManager->GetShadowInstanceIndexBuffer(frameIndex)->buffer->GetAddress();

		pushConstants.modelRenderIndicesBuffer = resourceManager->GetComponentBufferManager()->GetComponentBuffer("ModelRenderIndicesData", frameIndex)->buffer->GetAddress();
		pushConstants.pointLightShadowModelInstanceBufferAddresses = pointLightBufferManager->GetShadowModelInstanceBufferAddresses(frameIndex)->buffer->GetAddress();
		pushConstants.pointLightShadowModelIndirectDrawBufferAddresses = pointLightBufferManager->GetShadowModelIndirectDrawBufferAddresses(frameIndex)->buffer->GetAddress();

		pushConstants.shapeRenderIndicesBuffer = resourceManager->GetComponentBufferManager()->GetComponentBuffer("ShapeRenderIndicesData", frameIndex)->buffer->GetAddress();
		pushConstants.pointLightShadowShapeInstanceBufferAddresses = pointLightBufferManager->GetShadowShapeInstanceBufferAddresses(frameIndex)->buffer->GetAddress();
		pushConstants.pointLightShadowShapeIndirectDrawBufferAddresses = pointLightBufferManager->GetShadowShapeIndirectDrawBufferAddresses(frameIndex)->buffer->GetAddress();

		vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipeline->GetPipeline());
		vkCmdPushConstants(commandBuffer, pipeline->GetLayout(), VK_SHADER_STAGE_COMPUTE_BIT, 0, sizeof(CullingPointLightShadowObjectsPushConstants), &pushConstants);

		vkCmdDispatchIndirect(commandBuffer, pointLightBufferManager->GetShadowDispatchIndirectBuffers(frameIndex)->buffer->GetBuffer(), 0);
	}

	// Barrier: Wait for Pass 2 to fill the Indirect Draw Buffers for the actual Shadow Map Render Pass
	VkMemoryBarrier2 drawBarrier = { VK_STRUCTURE_TYPE_MEMORY_BARRIER_2 };
	drawBarrier.srcStageMask = VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT;
	drawBarrier.srcAccessMask = VK_ACCESS_2_SHADER_WRITE_BIT;
	drawBarrier.dstStageMask = VK_PIPELINE_STAGE_2_DRAW_INDIRECT_BIT | VK_PIPELINE_STAGE_2_VERTEX_SHADER_BIT;
	drawBarrier.dstAccessMask = VK_ACCESS_2_INDIRECT_COMMAND_READ_BIT | VK_ACCESS_2_SHADER_READ_BIT;

	VkDependencyInfo drawDepInfo = { VK_STRUCTURE_TYPE_DEPENDENCY_INFO };
	drawDepInfo.memoryBarrierCount = 1;
	drawDepInfo.pMemoryBarriers = &drawBarrier;
	vkCmdPipelineBarrier2(commandBuffer, &drawDepInfo);
}
*/