#include "ShadowRenderer.h"
#include "Engine/Components/PointLightComponent.h"
#include "Engine/Components/SpotLightComponent.h"
#include "Engine/Components/DirectionLightComponent.h"
#include "Engine/Render/GpuStructs.h"

void ShadowRenderer::Initialize(std::shared_ptr<ResourceManager> resourceManager)
{

}

void ShadowRenderer::Render(VkCommandBuffer commandBuffer, std::shared_ptr<Registry> registry, std::shared_ptr<ResourceManager> resourceManager, uint32_t frameIndex, std::function<void()> renderFunction)
{
	RenderPointLightShadows(commandBuffer, registry, resourceManager, frameIndex);
	RenderSpotLightShadows(commandBuffer, registry, resourceManager, frameIndex);
	RenderDirectionLightShadows(commandBuffer, registry, resourceManager, frameIndex);
}

void ShadowRenderer::RenderSpotLightShadows(VkCommandBuffer commandBuffer, std::shared_ptr<Registry> registry, std::shared_ptr<ResourceManager> resourceManager, uint32_t frameIndex)
{
	auto spotLightPool = registry->GetPool<SpotLightComponent>();
	auto spotLightShadowPool = registry->GetPool<SpotLightShadowComponent>();

	if (!spotLightPool || !spotLightShadowPool || spotLightShadowPool->GetDenseSize() == 0)
		return;

	auto vulkanContext = Vk::VulkanContext::GetContext();
	auto pipeline = resourceManager->GetVulkanManager()->GetGraphicsPipeline("ShadowSpotLight");
	auto shadowBufferManager = resourceManager->GetSpotLightShadowBufferManager();

	SpotLightShadowPushConstants pushConstants = {};
	pushConstants.transformBuffer = resourceManager->GetComponentBufferManager()->GetComponentBuffer("TransformData", frameIndex)->buffer->GetAddress();
	pushConstants.modelRenderIndicesBuffer = resourceManager->GetComponentBufferManager()->GetComponentBuffer("ModelRenderIndicesData", frameIndex)->buffer->GetAddress();
	pushConstants.modelBufferAddresses = resourceManager->GetModelManager()->GetDeviceAddressesBuffer(frameIndex)->buffer->GetAddress();
	pushConstants.animationTransformBufferAddresses = resourceManager->GetComponentBufferManager()->GetComponentBuffer("AnimationNodeTransformDeviceAddressesBuffers", frameIndex)->buffer->GetAddress();
	pushConstants.animationVertexBoneBufferAddresses = resourceManager->GetAnimationManager()->GetDeviceAddressesBuffer(frameIndex)->buffer->GetAddress();
	pushConstants.shapeRenderIndicesBuffer = resourceManager->GetComponentBufferManager()->GetComponentBuffer("ShapeRenderIndicesData", frameIndex)->buffer->GetAddress();
	pushConstants.shapeBufferAddresses = resourceManager->GetGeometryManager()->GetDeviceAddressesBuffer(frameIndex)->buffer->GetAddress();
	pushConstants.spotLightBufferAddress = resourceManager->GetComponentBufferManager()->GetComponentBuffer("SpotLightData", frameIndex)->buffer->GetAddress();
	pushConstants.spotLightShadowBufferAddress = resourceManager->GetComponentBufferManager()->GetComponentBuffer("SpotLightShadowData", frameIndex)->buffer->GetAddress();

	auto& denseIndices = spotLightShadowPool->GetDenseIndices();

	for (const auto& entity : denseIndices)
	{
		auto& shadowComponent = spotLightShadowPool->GetData(entity);
		uint32_t spotLightIndex = spotLightPool->GetDenseIndex(entity);
		uint32_t shadowIndex = spotLightShadowPool->GetDenseIndex(entity);
		auto& frameBufferObj = shadowComponent.frameBuffers[frameIndex].frameBuffer;

		if (!frameBufferObj) continue;

		Vk::Image::TransitionImageLayoutDynamic(
			commandBuffer,
			frameBufferObj->GetImage("Depth")->Value(),
			VK_IMAGE_LAYOUT_UNDEFINED,
			VK_PIPELINE_STAGE_2_TOP_OF_PIPE_BIT,
			VK_ACCESS_2_NONE,
			VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
			VK_PIPELINE_STAGE_2_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_2_LATE_FRAGMENT_TESTS_BIT,
			VK_ACCESS_2_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT
		);

		VkRenderingAttachmentInfo depthAttachment = Vk::DynamicRendering::BuildRenderingAttachmentInfo(
			frameBufferObj->GetImage("Depth")->GetImageView("Default"),
			VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
			nullptr
		);

		depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		depthAttachment.clearValue.depthStencil = { 1.0f, 0 };

		VkRenderingInfo renderingInfo = Vk::DynamicRendering::BuildRenderingInfo(frameBufferObj->GetSize(), {}, &depthAttachment);

		vkCmdBeginRendering(commandBuffer, &renderingInfo);
		vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline->GetPipeline());

		VkViewport viewport{ 0.0f, 0.0f, (float)frameBufferObj->GetSize().width, (float)frameBufferObj->GetSize().height, 0.0f, 1.0f };
		vkCmdSetViewport(commandBuffer, 0, 1, &viewport);
		VkRect2D scissor{ {0, 0}, frameBufferObj->GetSize() };
		vkCmdSetScissor(commandBuffer, 0, 1, &scissor);

		auto shadowRes = shadowBufferManager->GetShadowResources(frameIndex, shadowIndex);

		pushConstants.spotLightIndex = spotLightIndex;

		
		{//Rendering Models
			pushConstants.renderMode = MODEL_INSTANCED;
			pushConstants.instanceIndexAddressBuffer = shadowRes.modelInstanceAddressListBuffer->buffer->GetAddress();

			vkCmdPushConstants(commandBuffer, pipeline->GetLayout(), VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(SpotLightShadowPushConstants), &pushConstants);

			if (shadowRes.modelIndirectDrawBuffer && shadowRes.modelIndirectDrawBuffer->buffer)
			{
				vkCmdDrawIndirect(
					commandBuffer,
					shadowRes.modelIndirectDrawBuffer->buffer->Value(),
					0,
					resourceManager->GetModelManager()->GetCurrentCount(),
					sizeof(VkDrawIndirectCommand)
				);
			}
		}
		
		{ //Rendering Shapes
			pushConstants.renderMode = SHAPE_INSTANCED;
			pushConstants.instanceIndexAddressBuffer = shadowRes.shapeInstanceAddressListBuffer->buffer->GetAddress();

			vkCmdPushConstants(commandBuffer, pipeline->GetLayout(), VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(SpotLightShadowPushConstants), &pushConstants);

			if (shadowRes.shapeIndirectDrawBuffer && shadowRes.shapeIndirectDrawBuffer->buffer)
			{
				vkCmdDrawIndirect(
					commandBuffer,
					shadowRes.shapeIndirectDrawBuffer->buffer->Value(),
					0,
					resourceManager->GetGeometryManager()->GetCurrentCount(), // Max draw count
					sizeof(VkDrawIndirectCommand)
				);
			}
		}

		vkCmdEndRendering(commandBuffer);

		Vk::Image::TransitionImageLayoutDynamic(
			commandBuffer,
			frameBufferObj->GetImage("Depth")->Value(),
			VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
			VK_PIPELINE_STAGE_2_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_2_LATE_FRAGMENT_TESTS_BIT,
			VK_ACCESS_2_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
			VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL,
			VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT,
			VK_ACCESS_2_SHADER_READ_BIT
		);
	}
}

void ShadowRenderer::RenderPointLightShadows(VkCommandBuffer commandBuffer, std::shared_ptr<Registry> registry, std::shared_ptr<ResourceManager> resourceManager, uint32_t frameIndex)
{
	auto pointLightPool = registry->GetPool<PointLightComponent>();
	auto pointLightShadowPool = registry->GetPool<PointLightShadowComponent>();

	if (!pointLightPool || !pointLightShadowPool || pointLightShadowPool->GetDenseSize() == 0)
		return;

	auto vulkanContext = Vk::VulkanContext::GetContext();
	auto pipeline = resourceManager->GetVulkanManager()->GetGraphicsPipeline("ShadowPointLight");
	auto shadowBufferManager = resourceManager->GetPointLightShadowBufferManager();

	PointLightShadowPushConstants pushConstants = {};
	pushConstants.transformBuffer = resourceManager->GetComponentBufferManager()->GetComponentBuffer("TransformData", frameIndex)->buffer->GetAddress();
	pushConstants.modelRenderIndicesBuffer = resourceManager->GetComponentBufferManager()->GetComponentBuffer("ModelRenderIndicesData", frameIndex)->buffer->GetAddress();
	pushConstants.modelBufferAddresses = resourceManager->GetModelManager()->GetDeviceAddressesBuffer(frameIndex)->buffer->GetAddress();
	pushConstants.animationTransformBufferAddresses = resourceManager->GetComponentBufferManager()->GetComponentBuffer("AnimationNodeTransformDeviceAddressesBuffers", frameIndex)->buffer->GetAddress();
	pushConstants.animationVertexBoneBufferAddresses = resourceManager->GetAnimationManager()->GetDeviceAddressesBuffer(frameIndex)->buffer->GetAddress();
	pushConstants.shapeRenderIndicesBuffer = resourceManager->GetComponentBufferManager()->GetComponentBuffer("ShapeRenderIndicesData", frameIndex)->buffer->GetAddress();
	pushConstants.shapeBufferAddresses = resourceManager->GetGeometryManager()->GetDeviceAddressesBuffer(frameIndex)->buffer->GetAddress();
	pushConstants.pointLightBufferAddress = resourceManager->GetComponentBufferManager()->GetComponentBuffer("PointLightData", frameIndex)->buffer->GetAddress();
	pushConstants.pointLightShadowBufferAddress = resourceManager->GetComponentBufferManager()->GetComponentBuffer("PointLightShadowData", frameIndex)->buffer->GetAddress();

	auto& denseIndices = pointLightShadowPool->GetDenseIndices();

	for (const auto& entity : denseIndices)
	{
		auto& shadowComponent = pointLightShadowPool->GetData(entity);
		uint32_t pointLightIndex = pointLightPool->GetDenseIndex(entity);
		uint32_t shadowIndex = pointLightShadowPool->GetDenseIndex(entity);
		auto& frameBufferObj = shadowComponent.frameBuffers[frameIndex].frameBuffer;

		if (!frameBufferObj) continue;

		Vk::Image::TransitionImageLayoutDynamic(
			commandBuffer,
			frameBufferObj->GetImage("Depth")->Value(),
			VK_IMAGE_LAYOUT_UNDEFINED,
			VK_PIPELINE_STAGE_2_TOP_OF_PIPE_BIT,
			VK_ACCESS_2_NONE,
			VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
			VK_PIPELINE_STAGE_2_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_2_LATE_FRAGMENT_TESTS_BIT,
			VK_ACCESS_2_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT
		);

		VkRenderingAttachmentInfo depthAttachment = Vk::DynamicRendering::BuildRenderingAttachmentInfo(
			frameBufferObj->GetImage("Depth")->GetImageView("Default"),
			VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
			nullptr
		);

		depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		depthAttachment.clearValue.depthStencil = { 1.0f, 0 };

		VkRenderingInfo renderingInfo = Vk::DynamicRendering::BuildRenderingInfo(frameBufferObj->GetSize(), {}, &depthAttachment);
		renderingInfo.layerCount = 6;

		vkCmdBeginRendering(commandBuffer, &renderingInfo);
		vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline->GetPipeline());

		VkViewport viewport{ 0.0f, 0.0f, (float)frameBufferObj->GetSize().width, (float)frameBufferObj->GetSize().height, 0.0f, 1.0f };
		vkCmdSetViewport(commandBuffer, 0, 1, &viewport);
		VkRect2D scissor{ {0, 0}, frameBufferObj->GetSize() };
		vkCmdSetScissor(commandBuffer, 0, 1, &scissor);

		auto shadowRes = shadowBufferManager->GetShadowResources(frameIndex, shadowIndex);

		pushConstants.pointLightIndex = pointLightIndex;

		{
			pushConstants.renderMode = MODEL_INSTANCED;
			pushConstants.instanceIndexAddressBuffer = shadowRes.modelInstanceAddressListBuffer->buffer->GetAddress();

			vkCmdPushConstants(commandBuffer, pipeline->GetLayout(), VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_GEOMETRY_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(PointLightShadowPushConstants), &pushConstants);

			if (shadowRes.modelIndirectDrawBuffer && shadowRes.modelIndirectDrawBuffer->buffer)
			{
				vkCmdDrawIndirect(
					commandBuffer,
					shadowRes.modelIndirectDrawBuffer->buffer->Value(),
					0,
					resourceManager->GetModelManager()->GetCurrentCount(),
					sizeof(VkDrawIndirectCommand)
				);
			}
		}

		{
			pushConstants.renderMode = SHAPE_INSTANCED;
			pushConstants.instanceIndexAddressBuffer = shadowRes.shapeInstanceAddressListBuffer->buffer->GetAddress();

			vkCmdPushConstants(commandBuffer, pipeline->GetLayout(), VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_GEOMETRY_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(PointLightShadowPushConstants), &pushConstants);

			if (shadowRes.shapeIndirectDrawBuffer && shadowRes.shapeIndirectDrawBuffer->buffer)
			{
				vkCmdDrawIndirect(
					commandBuffer,
					shadowRes.shapeIndirectDrawBuffer->buffer->Value(),
					0,
					resourceManager->GetGeometryManager()->GetCurrentCount(),
					sizeof(VkDrawIndirectCommand)
				);
			}
		}

		vkCmdEndRendering(commandBuffer);

		Vk::Image::TransitionImageLayoutDynamic(
			commandBuffer,
			frameBufferObj->GetImage("Depth")->Value(),
			VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
			VK_PIPELINE_STAGE_2_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_2_LATE_FRAGMENT_TESTS_BIT,
			VK_ACCESS_2_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
			VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL,
			VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT,
			VK_ACCESS_2_SHADER_READ_BIT
		);
	}
}

void ShadowRenderer::RenderDirectionLightShadows(VkCommandBuffer commandBuffer, std::shared_ptr<Registry> registry, std::shared_ptr<ResourceManager> resourceManager, uint32_t frameIndex)
{

}