#include "DepthHierarchyBuilder.h"
#include "Engine/Render/GpuStructs.h"
#include "Engine/Vulkan/DescriptorSet.h"

void DepthHierarchyBuilder::Build(VkCommandBuffer commandBuffer, std::shared_ptr<Registry> registry, std::shared_ptr<ResourceManager> resourceManager, uint32_t frameIndex)
{
	CopyLinearDepthToDepthPyramid(commandBuffer, registry, resourceManager, frameIndex);
	BuildDepthHierarchy(commandBuffer, registry, resourceManager, frameIndex);
}

void DepthHierarchyBuilder::CopyLinearDepthToDepthPyramid(VkCommandBuffer commandBuffer, std::shared_ptr<Registry> registry, std::shared_ptr<ResourceManager> resourceManager, uint32_t frameIndex)
{
	auto vulkanContext = Vk::VulkanContext::GetContext();
	auto device = vulkanContext->GetDevice();
	auto graphicsQueue = device->GetQueue(Vk::QueueType::GRAPHICS);
	auto pipeline = resourceManager->GetVulkanManager()->GetComputePipeline("HizLinearDepth");
	auto frameBuffer = resourceManager->GetVulkanManager()->GetFrameDependentFrameBuffer("Main", frameIndex);
	auto pushDescriptor = resourceManager->GetVulkanManager()->GetPushDescriptorSet("HizPushSetLinearDepth");

	uint32_t depthPyramidLevels = Vk::Image::GetMipLevels(frameBuffer->GetSize().width, frameBuffer->GetSize().height);
	uint32_t depthWidth = frameBuffer->GetSize().width;
	uint32_t depthHeight = frameBuffer->GetSize().height;

	Vk::Image::TransitionImageLayoutDynamic(commandBuffer, frameBuffer->GetImage("Depth")->Value(),
		VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL, VK_PIPELINE_STAGE_2_LATE_FRAGMENT_TESTS_BIT, VK_ACCESS_2_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
		VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL, VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT, VK_ACCESS_2_SHADER_READ_BIT);

	//Transfer to the source image to shader READ layout
	Vk::Image::TransitionImageLayoutDynamic(commandBuffer, frameBuffer->GetImage("DepthPyramid")->Value(),
		VK_IMAGE_LAYOUT_UNDEFINED, VK_PIPELINE_STAGE_2_TOP_OF_PIPE_BIT, VK_ACCESS_2_NONE,
		VK_IMAGE_LAYOUT_GENERAL, VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT, VK_ACCESS_2_SHADER_READ_BIT | VK_ACCESS_2_SHADER_WRITE_BIT, depthPyramidLevels, 0);

	VkImageView sourceImageView = frameBuffer->GetImage("Depth")->GetImageView("Default");

	std::string mipImageViewName = "Default_mip_" + std::to_string(0);
	VkImageView destImageView = frameBuffer->GetImage("DepthPyramid")->GetImageView(mipImageViewName);

	VkDescriptorImageInfo dstTarget;
	dstTarget.sampler = nullptr;
	dstTarget.imageView = destImageView;
	dstTarget.imageLayout = VK_IMAGE_LAYOUT_GENERAL;

	VkDescriptorImageInfo srcTarget;
	srcTarget.sampler = resourceManager->GetVulkanManager()->GetSampler("Nearest")->Value();
	srcTarget.imageView = sourceImageView;
	srcTarget.imageLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;

	std::vector<VkWriteDescriptorSet> descriptorWrites =
	{
		Vk::DesciptorSetUtils::BuildWritePushDescriptorSetInfo(pushDescriptor->GetImageLayout("SrcImage"), srcTarget),
		Vk::DesciptorSetUtils::BuildWritePushDescriptorSetInfo(pushDescriptor->GetImageLayout("DstImage"), dstTarget),
	};

	HizLinearDepthPushConstants pushConstants;
	pushConstants.outImageSize = glm::vec2(depthWidth, depthHeight);
	pushConstants.cameraBuffer = resourceManager->GetComponentBufferManager()->GetComponentBuffer("CameraData", frameIndex)->buffer->GetAddress();
	pushConstants.cameraIndex = 0; //TODO: Main camera

	uint32_t workgroupSize = 32; // Matches shader local_size_x
	uint32_t workgroupCountX = static_cast<uint32_t>(std::ceil(depthWidth / (float)workgroupSize));
	uint32_t workgroupCountY = static_cast<uint32_t>(std::ceil(depthHeight / (float)workgroupSize));

	device->vkCmdPushDescriptorSetKHR(commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipeline->GetLayout(), 0, descriptorWrites.size(), descriptorWrites.data());
	vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipeline->GetPipeline());
	vkCmdPushConstants(commandBuffer, pipeline->GetLayout(), VK_SHADER_STAGE_COMPUTE_BIT, 0, sizeof(HizLinearDepthPushConstants), &pushConstants);
	vkCmdDispatch(commandBuffer, workgroupCountX, workgroupCountY, 1);

	Vk::Image::TransitionImageLayoutDynamic(commandBuffer, frameBuffer->GetImage("DepthPyramid")->Value(),
		VK_IMAGE_LAYOUT_GENERAL, VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT, VK_ACCESS_2_SHADER_WRITE_BIT,
		VK_IMAGE_LAYOUT_GENERAL, VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT, VK_ACCESS_2_SHADER_READ_BIT, 1, 0);
}

void DepthHierarchyBuilder::BuildDepthHierarchy(VkCommandBuffer commandBuffer, std::shared_ptr<Registry> registry, std::shared_ptr<ResourceManager> resourceManager, uint32_t frameIndex)
{
	//Important: Cannot blit from depth image into a r32f color image... Need to use compute shader instead
	//If depth is used to sample in geometry shader -> Need to be in shader read layout
	//Also need a mechanism, to store for each image its stage flags, access flags and layouts, so no need to track manually
	/*
	Vk::Image::TransitionImageLayoutDynamic(commandBuffer, frameBuffer->GetImage("Depth")->Value(),
		VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL, VK_PIPELINE_STAGE_2_TOP_OF_PIPE_BIT, VK_ACCESS_2_NONE,
		VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, VK_PIPELINE_STAGE_2_TRANSFER_BIT, VK_ACCESS_2_TRANSFER_READ_BIT);

	Vk::Image::TransitionImageLayoutDynamic(commandBuffer, frameBuffer->GetImage("DepthPyramid")->Value(),
		VK_IMAGE_LAYOUT_UNDEFINED, VK_PIPELINE_STAGE_2_TOP_OF_PIPE_BIT, VK_ACCESS_2_NONE,
		VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_PIPELINE_STAGE_2_TRANSFER_BIT, VK_ACCESS_2_TRANSFER_WRITE_BIT);

	Vk::Image::CopyImageToImageDynamic(commandBuffer, frameBuffer->GetImage("Depth")->Value(), frameBuffer->GetSize(), 0, frameBuffer->GetImage("DepthPyramid")->Value(), frameBuffer->GetSize(), 0);
	*/

	auto vulkanContext = Vk::VulkanContext::GetContext();
	auto device = vulkanContext->GetDevice();
	auto graphicsQueue = device->GetQueue(Vk::QueueType::GRAPHICS);
	auto pipeline = resourceManager->GetVulkanManager()->GetComputePipeline("Hiz");
	auto frameBuffer = resourceManager->GetVulkanManager()->GetFrameDependentFrameBuffer("Main", frameIndex);
	auto pushDescriptor = resourceManager->GetVulkanManager()->GetPushDescriptorSet("HizPushSet");
	
	uint32_t depthPyramidLevels = Vk::Image::GetMipLevels(frameBuffer->GetSize().width, frameBuffer->GetSize().height);

	if (depthPyramidLevels == 1)
		return;

	VkImageView sourceImageView = frameBuffer->GetImage("DepthPyramid")->GetImageView("Default_mip_0");

	uint32_t depthPyramidWidth = frameBuffer->GetSize().width;
	uint32_t depthPyramidHeight = frameBuffer->GetSize().height;

	for (uint32_t i = 1; i < depthPyramidLevels; ++i)
	{
		std::string mipImageViewName = "Default_mip_" + std::to_string(i);
		VkImageView destImageView = frameBuffer->GetImage("DepthPyramid")->GetImageView(mipImageViewName);

		VkDescriptorImageInfo dstTarget;
		dstTarget.sampler = nullptr;
		dstTarget.imageView = destImageView;
		dstTarget.imageLayout = VK_IMAGE_LAYOUT_GENERAL;

		VkDescriptorImageInfo srcTarget;
		srcTarget.sampler = resourceManager->GetVulkanManager()->GetSampler("MaxReduction")->Value();
		srcTarget.imageView = sourceImageView;
		srcTarget.imageLayout = VK_IMAGE_LAYOUT_GENERAL;

		//Potential problem with srcTarget in BuildWritePushDescriptorSetInfo image info???
		std::vector<VkWriteDescriptorSet> descriptorWrites =
		{
			Vk::DesciptorSetUtils::BuildWritePushDescriptorSetInfo(pushDescriptor->GetImageLayout("SrcImage"), srcTarget),
			Vk::DesciptorSetUtils::BuildWritePushDescriptorSetInfo(pushDescriptor->GetImageLayout("DstImage"), dstTarget),
		};

		if (depthPyramidWidth > 1) depthPyramidWidth /= 2;
		if (depthPyramidHeight > 1) depthPyramidHeight /= 2;

		HizPushConstants pushConstants;
		pushConstants.outImageSize = glm::vec2(depthPyramidWidth, depthPyramidHeight);

		uint32_t workgroupSize = 32; // Matches shader local_size_x
		uint32_t workgroupCountX = static_cast<uint32_t>(std::ceil(depthPyramidWidth / (float)workgroupSize));
		uint32_t workgroupCountY = static_cast<uint32_t>(std::ceil(depthPyramidHeight / (float)workgroupSize));

		device->vkCmdPushDescriptorSetKHR(commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipeline->GetLayout(), 0, descriptorWrites.size(), descriptorWrites.data());
		vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipeline->GetPipeline());
		vkCmdPushConstants(commandBuffer, pipeline->GetLayout(), VK_SHADER_STAGE_COMPUTE_BIT, 0, sizeof(HizPushConstants), &pushConstants);
		vkCmdDispatch(commandBuffer, workgroupCountX, workgroupCountY, 1);

		sourceImageView = destImageView;

		Vk::Image::TransitionImageLayoutDynamic(commandBuffer, frameBuffer->GetImage("DepthPyramid")->Value(),
			VK_IMAGE_LAYOUT_GENERAL, VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT, VK_ACCESS_2_SHADER_WRITE_BIT,
			VK_IMAGE_LAYOUT_GENERAL, VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT, VK_ACCESS_2_SHADER_READ_BIT, 1, i);
	}

	//Transfer to the source image to shader READ layout
	Vk::Image::TransitionImageLayoutDynamic(commandBuffer, frameBuffer->GetImage("DepthPyramid")->Value(),
		VK_IMAGE_LAYOUT_GENERAL, VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT, VK_ACCESS_2_SHADER_WRITE_BIT,
		VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT, VK_ACCESS_2_SHADER_READ_BIT);
}