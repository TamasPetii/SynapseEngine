#include "DepthHierarchyBuilder.h"

void DepthHierarchyBuilder::BuildDepthHierarchy(VkCommandBuffer commandBuffer, std::shared_ptr<Registry> registry, std::shared_ptr<ResourceManager> resourceManager, uint32_t frameIndex)
{
	auto vulkanContext = Vk::VulkanContext::GetContext();
	auto device = vulkanContext->GetDevice();
	auto graphicsQueue = device->GetQueue(Vk::QueueType::GRAPHICS);

	auto frameBuffer = resourceManager->GetVulkanManager()->GetFrameDependentFrameBuffer("Main", frameIndex);
	auto pipeline = resourceManager->GetVulkanManager()->GetGraphicsPipeline("Hiz");
	
	Vk::Image::TransitionImageLayoutDynamic(commandBuffer, frameBuffer->GetImage("Depth")->Value(),
		VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL, VK_PIPELINE_STAGE_2_TOP_OF_PIPE_BIT, VK_ACCESS_2_NONE,
		VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, VK_PIPELINE_STAGE_2_TRANSFER_BIT, VK_ACCESS_2_TRANSFER_READ_BIT);

	Vk::Image::TransitionImageLayoutDynamic(commandBuffer, frameBuffer->GetImage("DepthPyramid")->Value(),
		VK_IMAGE_LAYOUT_UNDEFINED, VK_PIPELINE_STAGE_2_TOP_OF_PIPE_BIT, VK_ACCESS_2_NONE,
		VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_PIPELINE_STAGE_2_TRANSFER_BIT, VK_ACCESS_2_TRANSFER_WRITE_BIT);

	Vk::Image::CopyImageToImageDynamic(commandBuffer, frameBuffer->GetImage("Depth")->Value(), frameBuffer->GetSize(), 0, frameBuffer->GetImage("DepthPyramid")->Value(), frameBuffer->GetSize(), 0);

	uint32_t depthPyramidLevels = Vk::Image::GetMipLevels(frameBuffer->GetSize().width, frameBuffer->GetSize().height);

	//Ez lehet rossz layout miatt majd, erre figyelni kell
	if (depthPyramidLevels == 1)
		return;

	//Transfer to the source image to shader READ layout
	Vk::Image::TransitionImageLayoutDynamic(commandBuffer, frameBuffer->GetImage("DepthPyramid")->Value(),
		VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_PIPELINE_STAGE_2_TRANSFER_BIT, VK_ACCESS_2_TRANSFER_WRITE_BIT,
		VK_IMAGE_LAYOUT_GENERAL, VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT, VK_ACCESS_2_SHADER_READ_BIT);

	//Transfer to general layout for compute shader write
	Vk::Image::TransitionImageLayoutDynamic(commandBuffer, frameBuffer->GetImage("DepthPyramid")->Value(),
		VK_IMAGE_LAYOUT_UNDEFINED, VK_PIPELINE_STAGE_2_TOP_OF_PIPE_BIT, VK_ACCESS_2_NONE,
		VK_IMAGE_LAYOUT_GENERAL, VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT, VK_ACCESS_2_SHADER_READ_BIT | VK_ACCESS_2_SHADER_WRITE_BIT, depthPyramidLevels - 1, 1);

	VkImageView sourceImageView = frameBuffer->GetImage("DepthPyramid")->GetImageView(); //Todo: MIP 0

	for (uint32_t i = 1; i < depthPyramidLevels; ++i)
	{
		VkImageView destImageView = frameBuffer->GetImage("DepthPyramid")->GetImageView(); //TODO: MIP i

		VkDescriptorImageInfo destTarget;
		destTarget.sampler = resourceManager->GetVulkanManager()->GetSampler("MaxReduction")->Value();
		destTarget.imageView = destImageView;
		destTarget.imageLayout = VK_IMAGE_LAYOUT_GENERAL;

		VkDescriptorImageInfo sourceTarget;
		sourceTarget.sampler = resourceManager->GetVulkanManager()->GetSampler("MaxReduction")->Value();
		sourceTarget.imageView = sourceImageView;
		sourceTarget.imageLayout = VK_IMAGE_LAYOUT_GENERAL;

		//...

		sourceImageView = destImageView;
	}

	//Transfer to the source image to shader READ layout
	Vk::Image::TransitionImageLayoutDynamic(commandBuffer, frameBuffer->GetImage("DepthPyramid")->Value(),
		VK_IMAGE_LAYOUT_GENERAL, VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT, VK_ACCESS_2_SHADER_WRITE_BIT,
		VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT, VK_ACCESS_2_SHADER_READ_BIT);
}