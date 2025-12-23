#include "BloomRenderer.h"
#include "Engine/Render/GpuStructs.h"
#include "Engine/Vulkan/DescriptorSet.h"

BloomSettings BloomRenderer::settings;

void BloomRenderer::Initialize(std::shared_ptr<ResourceManager> resourceManager)
{

}

void BloomRenderer::Render(VkCommandBuffer commandBuffer, std::shared_ptr<Registry> registry, std::shared_ptr<ResourceManager> resourceManager, uint32_t frameIndex, std::function<void()> renderFunction)
{
	if (!settings.enabled)
	{
		auto frameBuffer = resourceManager->GetVulkanManager()->GetFrameDependentFrameBuffer("Main", frameIndex);

		Vk::Image::TransitionImageLayoutDynamic(commandBuffer, frameBuffer->GetImage("Main")->Value(),
			VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT, VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT,
			VK_IMAGE_LAYOUT_GENERAL, VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT, VK_ACCESS_2_SHADER_READ_BIT | VK_ACCESS_2_SHADER_WRITE_BIT);

		return;
	}

	Prefilter(commandBuffer, resourceManager, frameIndex);


	if (GlobalConfig::BloomConfig::simulateDownSample)
		Downsample(commandBuffer, resourceManager, frameIndex);

	if (GlobalConfig::BloomConfig::simulateUpSample)
		Upsample(commandBuffer, resourceManager, frameIndex);

	Composite(commandBuffer, resourceManager, frameIndex);
}

void BloomRenderer::Prefilter(VkCommandBuffer cmd, std::shared_ptr<ResourceManager> rm, uint32_t frameIndex)
{
	auto vulkanContext = Vk::VulkanContext::GetContext();
	auto device = vulkanContext->GetDevice();
	auto pipeline = rm->GetVulkanManager()->GetComputePipeline("BloomPrefilter");
	auto pushDescriptor = rm->GetVulkanManager()->GetPushDescriptorSet("BloomPushSet");
	auto frameBuffer = rm->GetVulkanManager()->GetFrameDependentFrameBuffer("Main", frameIndex);

	VkImageView srcView = frameBuffer->GetImage("Main")->GetImageView("Default");
	VkImageView dstView = frameBuffer->GetImage("Bloom")->GetImageView("Default_mip_0");
	uint32_t width = frameBuffer->GetSize().width;
	uint32_t height = frameBuffer->GetSize().height;

	Vk::Image::TransitionImageLayoutDynamic(cmd, frameBuffer->GetImage("Bloom")->Value(),
		VK_IMAGE_LAYOUT_UNDEFINED, VK_PIPELINE_STAGE_2_TOP_OF_PIPE_BIT, VK_ACCESS_2_NONE,
		VK_IMAGE_LAYOUT_GENERAL, VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT, VK_ACCESS_2_SHADER_WRITE_BIT, 1, 0);

	Vk::Image::TransitionImageLayoutDynamic(cmd, frameBuffer->GetImage("Main")->Value(),
		VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT, VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT,
		VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT, VK_ACCESS_2_SHADER_READ_BIT);

	VkDescriptorImageInfo srcInfo = { .sampler = rm->GetVulkanManager()->GetSampler("BloomSampler")->Value(), .imageView = srcView, .imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL };
	VkDescriptorImageInfo dstInfo = { .sampler = nullptr, .imageView = dstView, .imageLayout = VK_IMAGE_LAYOUT_GENERAL };

	std::vector<VkWriteDescriptorSet> writes = {
		Vk::DesciptorSetUtils::BuildWritePushDescriptorSetInfo(pushDescriptor->GetImageLayout("SrcImage"), srcInfo),
		Vk::DesciptorSetUtils::BuildWritePushDescriptorSetInfo(pushDescriptor->GetImageLayout("DstImage"), dstInfo),
	};

	BloomPrefilterPushConstants pc;
	pc.texelSize = 1.0f / glm::vec2(width, height);
	pc.threshold = settings.threshold;
	pc.knee = settings.knee;

	vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_COMPUTE, pipeline->GetPipeline());
	device->vkCmdPushDescriptorSetKHR(cmd, VK_PIPELINE_BIND_POINT_COMPUTE, pipeline->GetLayout(), 0, writes.size(), writes.data());
	vkCmdPushConstants(cmd, pipeline->GetLayout(), VK_SHADER_STAGE_COMPUTE_BIT, 0, sizeof(pc), &pc);

	uint32_t workgroupSize = 8; // Matches shader local_size_x, local_size_y
	uint32_t workgroupCountX = static_cast<uint32_t>(std::ceil(width / (float)workgroupSize));
	uint32_t workgroupCountY = static_cast<uint32_t>(std::ceil(height / (float)workgroupSize));

	vkCmdDispatch(cmd, workgroupCountX, workgroupCountY, 1);
}

void BloomRenderer::Downsample(VkCommandBuffer cmd, std::shared_ptr<ResourceManager> rm, uint32_t frameIndex)
{
	auto vulkanContext = Vk::VulkanContext::GetContext();
	auto device = vulkanContext->GetDevice();
	auto pipeline = rm->GetVulkanManager()->GetComputePipeline("BloomDownsample");
	auto pushDescriptor = rm->GetVulkanManager()->GetPushDescriptorSet("BloomPushSet");
	auto frameBuffer = rm->GetVulkanManager()->GetFrameDependentFrameBuffer("Main", frameIndex);

	auto bloomImg = frameBuffer->GetImage("Bloom");
	uint32_t mips = Vk::Image::GetMipLevels(frameBuffer->GetSize().width, frameBuffer->GetSize().height);
	uint32_t width = frameBuffer->GetSize().width;
	uint32_t height = frameBuffer->GetSize().height;

	for (uint32_t i = 0; i < mips - 1; ++i)
	{
		uint32_t srcW = width >> i;
		uint32_t srcH = height >> i;
		uint32_t dstW = srcW >> 1;
		uint32_t dstH = srcH >> 1;

		if (dstW == 0) dstW = 1;
		if (dstH == 0) dstH = 1;

		Vk::Image::TransitionImageLayoutDynamic(cmd, bloomImg->Value(),
			VK_IMAGE_LAYOUT_GENERAL, VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT, VK_ACCESS_2_SHADER_WRITE_BIT,
			VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT, VK_ACCESS_2_SHADER_READ_BIT, 1, i);

		Vk::Image::TransitionImageLayoutDynamic(cmd, bloomImg->Value(),
			VK_IMAGE_LAYOUT_UNDEFINED, VK_PIPELINE_STAGE_2_TOP_OF_PIPE_BIT, VK_ACCESS_2_NONE,
			VK_IMAGE_LAYOUT_GENERAL, VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT, VK_ACCESS_2_SHADER_WRITE_BIT, 1, i + 1);

		VkDescriptorImageInfo srcInfo = { .sampler = rm->GetVulkanManager()->GetSampler("BloomSampler")->Value(), .imageView = bloomImg->GetImageView("Default_mip_" + std::to_string(i)), .imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL };
		VkDescriptorImageInfo dstInfo = { .sampler = nullptr, .imageView = bloomImg->GetImageView("Default_mip_" + std::to_string(i + 1)), .imageLayout = VK_IMAGE_LAYOUT_GENERAL };

		std::vector<VkWriteDescriptorSet> writes = {
			Vk::DesciptorSetUtils::BuildWritePushDescriptorSetInfo(pushDescriptor->GetImageLayout("SrcImage"), srcInfo),
			Vk::DesciptorSetUtils::BuildWritePushDescriptorSetInfo(pushDescriptor->GetImageLayout("DstImage"), dstInfo),
		};

		BloomDownsamplePushConstants pc;
		pc.texelSize = 1.0f / glm::vec2(srcW, srcH);

		vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_COMPUTE, pipeline->GetPipeline());
		device->vkCmdPushDescriptorSetKHR(cmd, VK_PIPELINE_BIND_POINT_COMPUTE, pipeline->GetLayout(), 0, writes.size(), writes.data());
		vkCmdPushConstants(cmd, pipeline->GetLayout(), VK_SHADER_STAGE_COMPUTE_BIT, 0, sizeof(pc), &pc);

		uint32_t workgroupSize = 8; // Matches shader local_size_x, local_size_y
		uint32_t workgroupCountX = static_cast<uint32_t>(std::ceil(dstW / (float)workgroupSize));
		uint32_t workgroupCountY = static_cast<uint32_t>(std::ceil(dstH / (float)workgroupSize));

		vkCmdDispatch(cmd, workgroupCountX, workgroupCountY, 1);
	}
}

void BloomRenderer::Upsample(VkCommandBuffer cmd, std::shared_ptr<ResourceManager> rm, uint32_t frameIndex)
{
	auto vulkanContext = Vk::VulkanContext::GetContext();
	auto device = vulkanContext->GetDevice();
	auto pipeline = rm->GetVulkanManager()->GetComputePipeline("BloomUpsample");
	auto pushDescriptor = rm->GetVulkanManager()->GetPushDescriptorSet("BloomPushSet");
	auto frameBuffer = rm->GetVulkanManager()->GetFrameDependentFrameBuffer("Main", frameIndex);

	auto bloomImg = frameBuffer->GetImage("Bloom");
	uint32_t mips = Vk::Image::GetMipLevels(frameBuffer->GetSize().width, frameBuffer->GetSize().height);
	uint32_t width = frameBuffer->GetSize().width;
	uint32_t height = frameBuffer->GetSize().height;

	for (int i = mips - 2; i >= 0; --i)
	{
		uint32_t dstW = width >> i;
		uint32_t dstH = height >> i;
		uint32_t srcW = dstW >> 1; // i+1
		uint32_t srcH = dstH >> 1;

		Vk::Image::TransitionImageLayoutDynamic(cmd, bloomImg->Value(),
			VK_IMAGE_LAYOUT_GENERAL, VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT, VK_ACCESS_2_SHADER_WRITE_BIT,
			VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT, VK_ACCESS_2_SHADER_READ_BIT, 1, i + 1);

		Vk::Image::TransitionImageLayoutDynamic(cmd, bloomImg->Value(),
			VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT, VK_ACCESS_2_SHADER_READ_BIT,
			VK_IMAGE_LAYOUT_GENERAL, VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT, VK_ACCESS_2_SHADER_READ_BIT | VK_ACCESS_2_SHADER_WRITE_BIT, 1, i);

		VkDescriptorImageInfo srcInfo = { .sampler = rm->GetVulkanManager()->GetSampler("BloomSampler")->Value(), .imageView = bloomImg->GetImageView("Default_mip_" + std::to_string(i + 1)), .imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL };
		VkDescriptorImageInfo dstInfo = { .sampler = nullptr, .imageView = bloomImg->GetImageView("Default_mip_" + std::to_string(i)), .imageLayout = VK_IMAGE_LAYOUT_GENERAL };

		std::vector<VkWriteDescriptorSet> writes = {
			Vk::DesciptorSetUtils::BuildWritePushDescriptorSetInfo(pushDescriptor->GetImageLayout("SrcImage"), srcInfo),
			Vk::DesciptorSetUtils::BuildWritePushDescriptorSetInfo(pushDescriptor->GetImageLayout("DstImage"), dstInfo),
		};

		BloomUpsamplePushConstants pc;
		pc.texelSize = 1.0f / glm::vec2(srcW, srcH);
		pc.filterRadius = settings.upsampleRadius;

		vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_COMPUTE, pipeline->GetPipeline());
		device->vkCmdPushDescriptorSetKHR(cmd, VK_PIPELINE_BIND_POINT_COMPUTE, pipeline->GetLayout(), 0, writes.size(), writes.data());
		vkCmdPushConstants(cmd, pipeline->GetLayout(), VK_SHADER_STAGE_COMPUTE_BIT, 0, sizeof(pc), &pc);

		uint32_t workgroupSize = 8; // Matches shader local_size_x, local_size_y
		uint32_t workgroupCountX = static_cast<uint32_t>(std::ceil(dstW / (float)workgroupSize));
		uint32_t workgroupCountY = static_cast<uint32_t>(std::ceil(dstH / (float)workgroupSize));

		vkCmdDispatch(cmd, workgroupCountX, workgroupCountY, 1);
	}
}

void BloomRenderer::Composite(VkCommandBuffer cmd, std::shared_ptr<ResourceManager> rm, uint32_t frameIndex)
{
	auto vulkanContext = Vk::VulkanContext::GetContext();
	auto device = vulkanContext->GetDevice();
	auto pipeline = rm->GetVulkanManager()->GetComputePipeline("BloomComposite");
	auto pushDescriptor = rm->GetVulkanManager()->GetPushDescriptorSet("BloomPushSet");
	auto frameBuffer = rm->GetVulkanManager()->GetFrameDependentFrameBuffer("Main", frameIndex);

	auto mainImg = frameBuffer->GetImage("Main");
	auto bloomImg = frameBuffer->GetImage("Bloom");

	uint32_t width = frameBuffer->GetSize().width;
	uint32_t height = frameBuffer->GetSize().height;

	Vk::Image::TransitionImageLayoutDynamic(cmd, bloomImg->Value(),
		VK_IMAGE_LAYOUT_GENERAL, VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT, VK_ACCESS_2_SHADER_WRITE_BIT | VK_ACCESS_2_SHADER_READ_BIT,
		VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT, VK_ACCESS_2_SHADER_READ_BIT, 1, 0);

	Vk::Image::TransitionImageLayoutDynamic(cmd, mainImg->Value(),
		VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT, VK_ACCESS_2_SHADER_READ_BIT,
		VK_IMAGE_LAYOUT_GENERAL, VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT, VK_ACCESS_2_SHADER_READ_BIT | VK_ACCESS_2_SHADER_WRITE_BIT);

	VkDescriptorImageInfo srcInfo = { .sampler = rm->GetVulkanManager()->GetSampler("BloomSampler")->Value(), .imageView = bloomImg->GetImageView("Default_mip_0"), .imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL };
	VkDescriptorImageInfo dstInfo = { .sampler = nullptr, .imageView = mainImg->GetImageView("Default"), .imageLayout = VK_IMAGE_LAYOUT_GENERAL };

	std::vector<VkWriteDescriptorSet> writes = {
		Vk::DesciptorSetUtils::BuildWritePushDescriptorSetInfo(pushDescriptor->GetImageLayout("SrcImage"), srcInfo),
		Vk::DesciptorSetUtils::BuildWritePushDescriptorSetInfo(pushDescriptor->GetImageLayout("DstImage"), dstInfo),
	};

	BloomCompositePushConstants pc;
	pc.exposure = settings.exposure;
	pc.bloomStrength = settings.bloomStrength;

	vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_COMPUTE, pipeline->GetPipeline());
	device->vkCmdPushDescriptorSetKHR(cmd, VK_PIPELINE_BIND_POINT_COMPUTE, pipeline->GetLayout(), 0, writes.size(), writes.data());
	vkCmdPushConstants(cmd, pipeline->GetLayout(), VK_SHADER_STAGE_COMPUTE_BIT, 0, sizeof(pc), &pc);

	uint32_t workgroupSize = 8; // Matches shader local_size_x, local_size_y
	uint32_t workgroupCountX = static_cast<uint32_t>(std::ceil(width / (float)workgroupSize));
	uint32_t workgroupCountY = static_cast<uint32_t>(std::ceil(height / (float)workgroupSize));

	vkCmdDispatch(cmd, workgroupCountX, workgroupCountY, 1);
}