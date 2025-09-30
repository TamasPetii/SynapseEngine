#include "GeometryRenderer.h"
#include "../GpuStructs.h"
#include <algorithm>
#include <execution>
#include "Engine/Components/AnimationComponent.h"

void GeometryRenderer::Initialize(std::shared_ptr<ResourceManager> resourceManager)
{
}

void GeometryRenderer::Render(VkCommandBuffer commandBuffer, std::shared_ptr<Registry> registry, std::shared_ptr<ResourceManager> resourceManager, uint32_t frameIndex, std::function<void()> renderFunction)
{
	auto vulkanContext = Vk::VulkanContext::GetContext();
	auto device = vulkanContext->GetDevice();
	auto graphicsQueue = device->GetQueue(Vk::QueueType::GRAPHICS);
	auto frameBuffer = resourceManager->GetVulkanManager()->GetFrameDependentFrameBuffer("Main", frameIndex);
	//auto pipeline = resourceManager->GetVulkanManager()->GetGraphicsPipeline("DeferredPre");
	auto pipeline = resourceManager->GetVulkanManager()->GetGraphicsPipeline("DeferredPreIndirect");

	VkClearValue colorClearValue{};
	colorClearValue.color.float32[0] = 0.f;
	colorClearValue.color.float32[1] = 0.f;
	colorClearValue.color.float32[2] = 0.f;
	colorClearValue.color.float32[3] = 1.f;

	VkClearValue entityClearValue{};
	entityClearValue.color.uint32[0] = NULL_ENTITY;

	VkClearValue depthClearValue{};
	depthClearValue.depthStencil.depth = 1.0f;
	depthClearValue.depthStencil.stencil = 0;

	Vk::Image::TransitionImageLayoutDynamic(commandBuffer, frameBuffer->GetImage("Position")->Value(),
		VK_IMAGE_LAYOUT_UNDEFINED, VK_PIPELINE_STAGE_2_TOP_OF_PIPE_BIT, VK_ACCESS_2_NONE,
		VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT, VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT);

	Vk::Image::TransitionImageLayoutDynamic(commandBuffer, frameBuffer->GetImage("Color")->Value(),
		VK_IMAGE_LAYOUT_UNDEFINED, VK_PIPELINE_STAGE_2_TOP_OF_PIPE_BIT, VK_ACCESS_2_NONE,
		VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT, VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT);

	Vk::Image::TransitionImageLayoutDynamic(commandBuffer, frameBuffer->GetImage("Normal")->Value(),
		VK_IMAGE_LAYOUT_UNDEFINED, VK_PIPELINE_STAGE_2_TOP_OF_PIPE_BIT, VK_ACCESS_2_NONE,
		VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT, VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT);

	Vk::Image::TransitionImageLayoutDynamic(commandBuffer, frameBuffer->GetImage("Entity")->Value(),
		VK_IMAGE_LAYOUT_UNDEFINED, VK_PIPELINE_STAGE_2_TOP_OF_PIPE_BIT, VK_ACCESS_2_NONE,
		VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT, VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT);

	Vk::Image::TransitionImageLayoutDynamic(commandBuffer, frameBuffer->GetImage("Depth")->Value(),
		VK_IMAGE_LAYOUT_UNDEFINED, VK_PIPELINE_STAGE_2_TOP_OF_PIPE_BIT, VK_ACCESS_2_NONE,
		VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL, VK_PIPELINE_STAGE_2_EARLY_FRAGMENT_TESTS_BIT, VK_ACCESS_2_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT);

	VkRenderingAttachmentInfo positionAttachment = Vk::DynamicRendering::BuildRenderingAttachmentInfo(frameBuffer->GetImage("Position")->GetImageView(), VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, &colorClearValue);
	VkRenderingAttachmentInfo colorAttachment = Vk::DynamicRendering::BuildRenderingAttachmentInfo(frameBuffer->GetImage("Color")->GetImageView(), VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, &colorClearValue);
	VkRenderingAttachmentInfo normalAttachment = Vk::DynamicRendering::BuildRenderingAttachmentInfo(frameBuffer->GetImage("Normal")->GetImageView(), VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, &colorClearValue);
	VkRenderingAttachmentInfo entityAttachment = Vk::DynamicRendering::BuildRenderingAttachmentInfo(frameBuffer->GetImage("Entity")->GetImageView(), VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, &entityClearValue);
	VkRenderingAttachmentInfo depthAttachment = Vk::DynamicRendering::BuildRenderingAttachmentInfo(frameBuffer->GetImage("Depth")->GetImageView(), VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL, &depthClearValue);

	std::vector<VkRenderingAttachmentInfo> renderTargetAttachments = { positionAttachment, colorAttachment , normalAttachment, entityAttachment };
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

	//RenderShapesInstanced(commandBuffer, pipeline->GetLayout(), resourceManager, frameIndex);
	//RenderModelsInstanced(commandBuffer, pipeline->GetLayout(), resourceManager, frameIndex);
	RenderShapesInstancedIndirect(commandBuffer, pipeline->GetLayout(), resourceManager, frameIndex);
	RenderModelsInstancedIndirect(commandBuffer, pipeline->GetLayout(), resourceManager, frameIndex);

	//RenderShapesInstancedNew(commandBuffer, pipeline->GetLayout(), resourceManager, frameIndex);
	//RenderModelsInstancedNew(commandBuffer, pipeline->GetLayout(), resourceManager, frameIndex);

	vkCmdEndRendering(commandBuffer);
}

void GeometryRenderer::RenderShapesInstanced(VkCommandBuffer commandBuffer, VkPipelineLayout pipelineLayout, std::shared_ptr<ResourceManager> resourceManager, uint32_t frameIndex)
{
	auto geometryManager = resourceManager->GetGeometryManager();
	std::for_each(std::execution::seq, geometryManager->GetShapes().begin(), geometryManager->GetShapes().end(),
		[&](const auto& data) -> void 
		{
			if (data.second == nullptr)
				return;

			auto shape = data.second->object;

			if (shape->GetInstanceCount() > 0)
			{
				GeometryRendererPushConstants pushConstants;
				pushConstants.renderMode = SHAPE_INSTANCED;
				pushConstants.cameraIndex = 0;
				pushConstants.cameraBuffer = resourceManager->GetComponentBufferManager()->GetComponentBuffer("CameraData", frameIndex)->buffer->GetAddress();
				pushConstants.transformBuffer = resourceManager->GetComponentBufferManager()->GetComponentBuffer("TransformData", frameIndex)->buffer->GetAddress();
				pushConstants.instanceIndexBuffer = shape->GetInstanceIndexBuffer(frameIndex)->GetAddress(); // This could be included into model device addresses -> Better for gpu driven rendering
				pushConstants.shapeRenderIndicesBuffer = resourceManager->GetComponentBufferManager()->GetComponentBuffer("ShapeRenderIndicesData", frameIndex)->buffer->GetAddress();
				pushConstants.shapeBufferAddresses = resourceManager->GetGeometryManager()->GetDeviceAddressesBuffer(frameIndex)->buffer->GetAddress();
				pushConstants.shapeMaterialIndicesBuffer = resourceManager->GetComponentBufferManager()->GetComponentBuffer("MaterialData", frameIndex)->buffer->GetAddress();
				pushConstants.materialBuffer = resourceManager->GetMaterialManager()->GetDeviceAddressesBuffer(frameIndex)->buffer->GetAddress();


				vkCmdPushConstants(commandBuffer, pipelineLayout, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(GeometryRendererPushConstants), &pushConstants);
				vkCmdDraw(commandBuffer, shape->GetIndexCount(), shape->GetInstanceCount(), 0, 0);
			}
		}
	);
}

void GeometryRenderer::RenderModelsInstanced(VkCommandBuffer commandBuffer, VkPipelineLayout pipelineLayout, std::shared_ptr<ResourceManager> resourceManager, uint32_t frameIndex)
{
	auto modelManager = resourceManager->GetModelManager();
	std::for_each(std::execution::seq, modelManager->GetModels().begin(), modelManager->GetModels().end(),
		[&](const auto& data) -> void 
		{
			if (data.second == nullptr)
				return;

			auto model = data.second->object;

			if (model && model->state == LoadState::Ready && model->GetInstanceCount() > 0)
			{
				//Global buffers could be uploaded once / model pass
				GeometryRendererPushConstants pushConstants;
				pushConstants.renderMode = MODEL_INSTANCED;
				pushConstants.cameraIndex = 0;
				pushConstants.cameraBuffer = resourceManager->GetComponentBufferManager()->GetComponentBuffer("CameraData", frameIndex)->buffer->GetAddress();
				pushConstants.transformBuffer = resourceManager->GetComponentBufferManager()->GetComponentBuffer("TransformData", frameIndex)->buffer->GetAddress();
				pushConstants.instanceIndexBuffer = model->GetInstanceIndexBuffer(frameIndex)->GetAddress(); // This could be included into model device addresses -> Better for gpu driven rendering
				pushConstants.modelRenderIndicesBuffer = resourceManager->GetComponentBufferManager()->GetComponentBuffer("ModelRenderIndicesData", frameIndex)->buffer->GetAddress();
				pushConstants.modelBufferAddresses = resourceManager->GetModelManager()->GetDeviceAddressesBuffer(frameIndex)->buffer->GetAddress();
				pushConstants.animationTransformBufferAddresses = resourceManager->GetComponentBufferManager()->GetComponentBuffer("AnimationNodeTransformDeviceAddressesBuffers", frameIndex)->buffer->GetAddress();
				pushConstants.animationVertexBoneBufferAddresses = resourceManager->GetAnimationManager()->GetDeviceAddressesBuffer(frameIndex)->buffer->GetAddress();
				pushConstants.materialBuffer = resourceManager->GetMaterialManager()->GetDeviceAddressesBuffer(frameIndex)->buffer->GetAddress();

				vkCmdPushConstants(commandBuffer, pipelineLayout, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(GeometryRendererPushConstants), &pushConstants);					
				vkCmdDraw(commandBuffer, model->GetIndexCount(), model->GetInstanceCount(), 0, 0);
			}
		}
	);
}

void GeometryRenderer::RenderShapesInstancedNew(VkCommandBuffer commandBuffer, VkPipelineLayout pipelineLayout, std::shared_ptr<ResourceManager> resourceManager, uint32_t frameIndex)
{
	auto geometryManager = resourceManager->GetGeometryManager();
	std::for_each(std::execution::seq, geometryManager->GetShapes().begin(), geometryManager->GetShapes().end(),
		[&](const auto& data) -> void
		{
			if (data.second == nullptr)
				return;

			auto shape = data.second->object;

			if (shape->GetInstanceCount() > 0)
			{
				GeometryRendererPushConstants pushConstants;
				pushConstants.renderMode = SHAPE_INSTANCED;
				pushConstants.cameraIndex = 0;
				pushConstants.cameraBuffer = resourceManager->GetComponentBufferManager()->GetComponentBuffer("CameraData", frameIndex)->buffer->GetAddress();
				pushConstants.transformBuffer = resourceManager->GetComponentBufferManager()->GetComponentBuffer("TransformData", frameIndex)->buffer->GetAddress();
				pushConstants.instanceIndexBuffer = geometryManager->GetInstanceIndexAddressBuffer(frameIndex)->buffer->GetAddress();
				pushConstants.shapeRenderIndicesBuffer = resourceManager->GetComponentBufferManager()->GetComponentBuffer("ShapeRenderIndicesData", frameIndex)->buffer->GetAddress();
				pushConstants.shapeBufferAddresses = resourceManager->GetGeometryManager()->GetDeviceAddressesBuffer(frameIndex)->buffer->GetAddress();
				pushConstants.shapeMaterialIndicesBuffer = resourceManager->GetComponentBufferManager()->GetComponentBuffer("MaterialData", frameIndex)->buffer->GetAddress();
				pushConstants.materialBuffer = resourceManager->GetMaterialManager()->GetDeviceAddressesBuffer(frameIndex)->buffer->GetAddress();
				pushConstants.objectIndex = shape->GetBufferArrayIndex();

				vkCmdPushConstants(commandBuffer, pipelineLayout, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(GeometryRendererPushConstants), &pushConstants);
				vkCmdDraw(commandBuffer, shape->GetIndexCount(), shape->GetInstanceCount(), 0, 0);
			}
		}
	);
}

void GeometryRenderer::RenderModelsInstancedNew(VkCommandBuffer commandBuffer, VkPipelineLayout pipelineLayout, std::shared_ptr<ResourceManager> resourceManager, uint32_t frameIndex)
{
	auto modelManager = resourceManager->GetModelManager();
	std::for_each(std::execution::seq, modelManager->GetModels().begin(), modelManager->GetModels().end(),
		[&](const auto& data) -> void
		{
			if (data.second == nullptr)
				return;

			auto model = data.second->object;

			if (model && model->state == LoadState::Ready && model->GetInstanceCount() > 0)
			{
				//Global buffers could be uploaded once / model pass
				GeometryRendererPushConstants pushConstants;
				pushConstants.renderMode = MODEL_INSTANCED;
				pushConstants.cameraIndex = 0;
				pushConstants.cameraBuffer = resourceManager->GetComponentBufferManager()->GetComponentBuffer("CameraData", frameIndex)->buffer->GetAddress();
				pushConstants.transformBuffer = resourceManager->GetComponentBufferManager()->GetComponentBuffer("TransformData", frameIndex)->buffer->GetAddress();
				pushConstants.instanceIndexBuffer = modelManager->GetInstanceIndexAddressBuffer(frameIndex)->buffer->GetAddress();
				pushConstants.modelRenderIndicesBuffer = resourceManager->GetComponentBufferManager()->GetComponentBuffer("ModelRenderIndicesData", frameIndex)->buffer->GetAddress();
				pushConstants.modelBufferAddresses = resourceManager->GetModelManager()->GetDeviceAddressesBuffer(frameIndex)->buffer->GetAddress();
				pushConstants.animationTransformBufferAddresses = resourceManager->GetComponentBufferManager()->GetComponentBuffer("AnimationNodeTransformDeviceAddressesBuffers", frameIndex)->buffer->GetAddress();
				pushConstants.animationVertexBoneBufferAddresses = resourceManager->GetAnimationManager()->GetDeviceAddressesBuffer(frameIndex)->buffer->GetAddress();
				pushConstants.materialBuffer = resourceManager->GetMaterialManager()->GetDeviceAddressesBuffer(frameIndex)->buffer->GetAddress();
				pushConstants.objectIndex = model->GetBufferArrayIndex();

				vkCmdPushConstants(commandBuffer, pipelineLayout, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(GeometryRendererPushConstants), &pushConstants);
				vkCmdDraw(commandBuffer, model->GetIndexCount(), model->GetInstanceCount(), 0, 0);
			}
		}
	);
}

void GeometryRenderer::RenderShapesInstancedIndirect(VkCommandBuffer commandBuffer, VkPipelineLayout pipelineLayout, std::shared_ptr<ResourceManager> resourceManager, uint32_t frameIndex)
{
	auto geometryManager = resourceManager->GetGeometryManager();

	GeometryRendererIndirectPushConstants pushConstants;
	pushConstants.renderMode = SHAPE_INSTANCED;
	pushConstants.cameraIndex = 0;
	pushConstants.cameraBuffer = resourceManager->GetComponentBufferManager()->GetComponentBuffer("CameraData", frameIndex)->buffer->GetAddress();
	pushConstants.transformBuffer = resourceManager->GetComponentBufferManager()->GetComponentBuffer("TransformData", frameIndex)->buffer->GetAddress();
	pushConstants.instanceIndexAddressBuffer = geometryManager->GetInstanceIndexAddressBuffer(frameIndex)->buffer->GetAddress();
	pushConstants.shapeRenderIndicesBuffer = resourceManager->GetComponentBufferManager()->GetComponentBuffer("ShapeRenderIndicesData", frameIndex)->buffer->GetAddress();
	pushConstants.shapeBufferAddresses = geometryManager->GetDeviceAddressesBuffer(frameIndex)->buffer->GetAddress();
	pushConstants.shapeMaterialIndicesBuffer = resourceManager->GetComponentBufferManager()->GetComponentBuffer("MaterialData", frameIndex)->buffer->GetAddress();
	pushConstants.materialBuffer = resourceManager->GetMaterialManager()->GetDeviceAddressesBuffer(frameIndex)->buffer->GetAddress();

	VkDeviceSize bufferSize = sizeof(uint32_t);
	uint32_t shapeCount = geometryManager->GetCurrentCount();
	auto shapeIndirectCountBuffer = geometryManager->GetIndirectCountBuffers(frameIndex);
	auto shapeIndirectCountBufferHandler = shapeIndirectCountBuffer->GetHandler();
	memcpy(shapeIndirectCountBufferHandler, &shapeCount, (size_t)bufferSize);

	vkCmdPushConstants(commandBuffer, pipelineLayout, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(GeometryRendererIndirectPushConstants), &pushConstants);
	vkCmdDrawIndirectCount(commandBuffer, geometryManager->GetIndirectDrawBuffer(frameIndex)->buffer->Value(), 0, shapeIndirectCountBuffer->Value(), 0, geometryManager->GetIndirectDrawBuffer(frameIndex)->size, sizeof(VkDrawIndirectCommand));
}

void GeometryRenderer::RenderModelsInstancedIndirect(VkCommandBuffer commandBuffer, VkPipelineLayout pipelineLayout, std::shared_ptr<ResourceManager> resourceManager, uint32_t frameIndex)
{
	auto modelManager = resourceManager->GetModelManager();

	GeometryRendererIndirectPushConstants pushConstants;
	pushConstants.renderMode = MODEL_INSTANCED;
	pushConstants.cameraIndex = 0;
	pushConstants.cameraBuffer = resourceManager->GetComponentBufferManager()->GetComponentBuffer("CameraData", frameIndex)->buffer->GetAddress();
	pushConstants.transformBuffer = resourceManager->GetComponentBufferManager()->GetComponentBuffer("TransformData", frameIndex)->buffer->GetAddress();
	pushConstants.instanceIndexAddressBuffer = modelManager->GetInstanceIndexAddressBuffer(frameIndex)->buffer->GetAddress();
	pushConstants.modelRenderIndicesBuffer = resourceManager->GetComponentBufferManager()->GetComponentBuffer("ModelRenderIndicesData", frameIndex)->buffer->GetAddress();
	pushConstants.modelBufferAddresses = modelManager->GetDeviceAddressesBuffer(frameIndex)->buffer->GetAddress();
	pushConstants.animationTransformBufferAddresses = resourceManager->GetComponentBufferManager()->GetComponentBuffer("AnimationNodeTransformDeviceAddressesBuffers", frameIndex)->buffer->GetAddress();
	pushConstants.animationVertexBoneBufferAddresses = resourceManager->GetAnimationManager()->GetDeviceAddressesBuffer(frameIndex)->buffer->GetAddress();
	pushConstants.materialBuffer = resourceManager->GetMaterialManager()->GetDeviceAddressesBuffer(frameIndex)->buffer->GetAddress();

	VkDeviceSize bufferSize = sizeof(uint32_t);
	uint32_t modelCount = modelManager->GetCurrentCount();
	auto modelIndirectCountBuffer = modelManager->GetIndirectCountBuffers(frameIndex);
	auto modelIndirectCountBufferHandler = modelIndirectCountBuffer->GetHandler();
	memcpy(modelIndirectCountBufferHandler, &modelCount, (size_t)bufferSize);

	/*
	{ //Debug
		std::cout << std::format("[GeometryRenderer::RenderModelsInstancedIndirect] : FrameIndex {}", frameIndex) << std::endl;

		uint32_t loadedModelCount = modelManager->GetCurrentCount();
		auto instanceIndexAddressBuffer = static_cast<VkDeviceAddress*>(modelManager->GetInstanceIndexAddressBuffer(frameIndex)->buffer->GetHandler());
		auto indirectCommandBufferHandler = static_cast<VkDrawIndirectCommand*>(modelManager->GetIndirectDrawBuffer(frameIndex)->buffer->GetHandler());

		//Print Instance Device Buffer Addresses
		for (uint32_t i = 0; i < loadedModelCount; ++i)
		{
			std::cout << std::format("[IndirectCommandBuffer][{}] - {} {} {} {}\n", i, indirectCommandBufferHandler[i].vertexCount, indirectCommandBufferHandler[i].instanceCount, indirectCommandBufferHandler[i].firstInstance, indirectCommandBufferHandler[i].firstVertex);
			std::cout << std::format("[InstanceIndexAddressBuffer][{}] - {}\n", i, instanceIndexAddressBuffer[i]);
			
			if (indirectCommandBufferHandler[i].instanceCount > 0)
				int x;

			auto instanceIndexBuffer = modelManager->GetInstanceIndexBuffer(frameIndex, i)->buffer;
			std::cout << std::format("[InstanceIndexBuffer][{}] - {}\n", i, instanceIndexBuffer->GetAddress());

			std::cout << std::format("[InstanceIndexBuffer][{}] - ", i);

			auto instanceIndexBufferHanlder = static_cast<uint32_t*>(instanceIndexBuffer->GetHandler());

			for (int j = 0; j < modelManager->GetInstanceIndexBuffer(frameIndex, i)->size; ++j)
			{
				std::cout << instanceIndexBufferHanlder[j] << " ";
			}

			std::cout << std::endl;
		}
	}
	*/

	vkCmdPushConstants(commandBuffer, pipelineLayout, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(GeometryRendererIndirectPushConstants), &pushConstants);
	vkCmdDrawIndirectCount(commandBuffer, modelManager->GetIndirectDrawBuffer(frameIndex)->buffer->Value(), 0, modelIndirectCountBuffer->Value(), 0, modelManager->GetIndirectDrawBuffer(frameIndex)->size, sizeof(VkDrawIndirectCommand));
}
