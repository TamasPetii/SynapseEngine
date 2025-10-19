#include "Image.h"
#include "Memory.h"

void Vk::ImageSpecification::AddImageViewConfig(const std::string& name, const ImageViewConfig& imageViewConfig)
{
	imageViewConfigs.insert(std::make_pair(name, imageViewConfig));
}

void Vk::ImageSpecification::AddImageViewConfig(const std::string& name, VkImageViewType viewType, uint32_t baseMipLevel, uint32_t mipLevelCount, std::optional<VkComponentMapping> swizzle, bool calcualteMipLevelAutomaticly)
{
	imageViewConfigs[name] = ImageViewConfig{ calcualteMipLevelAutomaticly, viewType, baseMipLevel, mipLevelCount, swizzle };
}

Vk::Image::Image(const ImageSpecification& specification) :
	specification(specification)
{
	Init();
}

Vk::Image::~Image()
{
	Destroy();
}

const VkImage Vk::Image::Value() const
{
	return image;
}

const VkImageView Vk::Image::GetImageView(const std::string& imageViewName) const
{
	if(imageViews.find(imageViewName) == imageViews.end())
		return VK_NULL_HANDLE;;

	return imageViews.at(imageViewName);
}

void Vk::Image::TransitionImageLayout(VkCommandBuffer commandBuffer, VkImage image, VkImageLayout srcLayout, VkPipelineStageFlags srcStageFlags, VkAccessFlags srcAccessMask, VkImageLayout dstLayout, VkPipelineStageFlags dstStageFlags, VkAccessFlags dstAccessMask, uint32_t mipmapLevel)
{
	VkImageMemoryBarrier barrier{};
	barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	barrier.oldLayout = srcLayout;
	barrier.newLayout = dstLayout;
	barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.image = image;

	barrier.subresourceRange.aspectMask = (dstLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL ? VK_IMAGE_ASPECT_DEPTH_BIT : VK_IMAGE_ASPECT_COLOR_BIT);
	barrier.subresourceRange.baseMipLevel = 0;
	barrier.subresourceRange.levelCount = 1;
	barrier.subresourceRange.baseArrayLayer = 0;
	barrier.subresourceRange.layerCount = 1;

	barrier.srcAccessMask = srcAccessMask;
	barrier.dstAccessMask = dstAccessMask;

	VkPipelineStageFlags sourceStage = srcStageFlags;
	VkPipelineStageFlags destinationStage = dstStageFlags;

	vkCmdPipelineBarrier(
		commandBuffer,
		sourceStage, destinationStage,
		0,
		0, nullptr,
		0, nullptr,
		1, &barrier
	);
}

void Vk::Image::CopyImageToImageDynamic(VkCommandBuffer commandBuffer, VkImage srcImage, VkExtent2D srcSize, uint32_t srcBaseMipMap, VkImage dstImage, VkExtent2D dstSize, uint32_t dstBaseMipMap)
{
	VkImageBlit2 imageBlitInfo{};
	imageBlitInfo.sType = VK_STRUCTURE_TYPE_IMAGE_BLIT_2;
	imageBlitInfo.srcOffsets[0] = { 0, 0, 0 };
	imageBlitInfo.srcOffsets[1] = { (int32_t)srcSize.width, (int32_t)srcSize.height, 1 };
	imageBlitInfo.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	imageBlitInfo.srcSubresource.mipLevel = srcBaseMipMap;
	imageBlitInfo.srcSubresource.baseArrayLayer = 0;
	imageBlitInfo.srcSubresource.layerCount = 1;

	imageBlitInfo.dstOffsets[0] = { 0, 0, 0 };
	imageBlitInfo.dstOffsets[1] = { (int32_t)dstSize.width, (int32_t)dstSize.height, 1 };
	imageBlitInfo.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	imageBlitInfo.dstSubresource.mipLevel = dstBaseMipMap;
	imageBlitInfo.dstSubresource.baseArrayLayer = 0;
	imageBlitInfo.dstSubresource.layerCount = 1;

	VkBlitImageInfo2 blitInfo{};
	blitInfo.sType = VK_STRUCTURE_TYPE_BLIT_IMAGE_INFO_2;
	blitInfo.srcImage = srcImage;
	blitInfo.srcImageLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
	blitInfo.dstImage = dstImage;
	blitInfo.dstImageLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
	blitInfo.filter = VK_FILTER_LINEAR;
	blitInfo.regionCount = 1;
	blitInfo.pRegions = &imageBlitInfo;

	vkCmdBlitImage2(commandBuffer, &blitInfo);
}

void Vk::Image::TransitionImageLayoutDynamic(VkCommandBuffer commandBuffer, VkImage image, VkImageLayout srcLayout, VkPipelineStageFlags2 srcStageFlags, VkAccessFlags2 srcAccessMask, VkImageLayout dstLayout, VkPipelineStageFlags2 dstStageFlags, VkAccessFlags2 dstAccessMask, uint32_t mipmapLevel, uint32_t baseMipMap)
{
	VkImageMemoryBarrier2 imageBarrier{};
	imageBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2;
	imageBarrier.oldLayout = srcLayout;
	imageBarrier.newLayout = dstLayout;
	imageBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	imageBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	imageBarrier.image = image;

	imageBarrier.subresourceRange.aspectMask = (dstLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL || dstLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL ? VK_IMAGE_ASPECT_DEPTH_BIT : VK_IMAGE_ASPECT_COLOR_BIT);
	imageBarrier.subresourceRange.baseMipLevel = baseMipMap;
	imageBarrier.subresourceRange.levelCount = mipmapLevel;
	imageBarrier.subresourceRange.baseArrayLayer = 0;
	imageBarrier.subresourceRange.layerCount = 1;

	imageBarrier.srcStageMask = srcStageFlags;
	imageBarrier.srcAccessMask = srcAccessMask;
	imageBarrier.dstStageMask = dstStageFlags;
	imageBarrier.dstAccessMask = dstAccessMask;

	VkDependencyInfo dependencyInfo{};
	dependencyInfo.sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO;
	dependencyInfo.dependencyFlags = 0;
	dependencyInfo.imageMemoryBarrierCount = 1;
	dependencyInfo.pImageMemoryBarriers = &imageBarrier;

	vkCmdPipelineBarrier2(commandBuffer, &dependencyInfo);
}

void Vk::Image::GenerateMipMaps(VkCommandBuffer commandBuffer, VkImage image, uint32_t width, uint32_t height, uint32_t mipmapLevels)
{
	uint32_t mipWidth = width;
	uint32_t mipHeight = height;

	uint32_t mipmapLevel = 1;

	for (uint32_t i = 1; i < mipmapLevels; i++)
	{
		uint32_t srcBaseMipMap = i - 1;
		uint32_t dstBaseMipMap = i;

		Image::TransitionImageLayoutDynamic(commandBuffer, image,
			VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_PIPELINE_STAGE_2_TRANSFER_BIT, VK_ACCESS_2_TRANSFER_WRITE_BIT,
			VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, VK_PIPELINE_STAGE_2_TRANSFER_BIT, VK_ACCESS_2_TRANSFER_READ_BIT,
			mipmapLevel, srcBaseMipMap);

		Image::CopyImageToImageDynamic(commandBuffer,
			image, { mipWidth, mipHeight }, srcBaseMipMap,
			image, { mipWidth > 1 ? mipWidth / 2 : 1, mipHeight > 1 ? mipHeight / 2 : 1, }, dstBaseMipMap);

		Image::TransitionImageLayoutDynamic(commandBuffer, image,
			VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, VK_PIPELINE_STAGE_2_TRANSFER_BIT, VK_ACCESS_2_TRANSFER_READ_BIT,
			VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT, VK_ACCESS_2_SHADER_READ_BIT,
			mipmapLevel, srcBaseMipMap);

		if (mipWidth > 1) mipWidth /= 2;
		if (mipHeight > 1) mipHeight /= 2;
	}

	Image::TransitionImageLayoutDynamic(commandBuffer, image,
		VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_PIPELINE_STAGE_2_TRANSFER_BIT, VK_ACCESS_2_TRANSFER_WRITE_BIT,
		VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT, VK_ACCESS_2_SHADER_READ_BIT,
		mipmapLevel, mipmapLevels - 1);
}

bool Vk::Image::ImageFormatSupportsLinearMipMap(VkFormat format)
{
	auto phyiscalDevice = VulkanContext::GetContext()->GetPhysicalDevice();
	VkFormatProperties formatProperties;
	vkGetPhysicalDeviceFormatProperties(phyiscalDevice->Value(), format, &formatProperties);

	return formatProperties.optimalTilingFeatures & VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT;
}

uint32_t Vk::Image::GetMipLevels(uint32_t width, uint32_t height)
{
	return static_cast<uint32_t>(std::floor(std::log2(std::max(width, height)))) + 1;
}

bool Vk::Image::IsDepthFormat(VkFormat format)
{
	switch (format)
	{
	case VK_FORMAT_D16_UNORM:
	case VK_FORMAT_D32_SFLOAT:
	case VK_FORMAT_X8_D24_UNORM_PACK32:
	case VK_FORMAT_D16_UNORM_S8_UINT:
	case VK_FORMAT_D24_UNORM_S8_UINT:
	case VK_FORMAT_D32_SFLOAT_S8_UINT:
		return true;
	default:
		return false;
	}
}

void Vk::Image::CopyImageToImage(VkCommandBuffer commandBuffer, VkImage srcImage, VkExtent2D srcSize, VkImage dstImage, VkExtent2D dstSize)
{
	VkImageBlit imageBlitInfo{};
	imageBlitInfo.srcOffsets[0] = { 0, 0, 0 };
	imageBlitInfo.srcOffsets[1] = { (int32_t)srcSize.width, (int32_t)srcSize.height, 1 };
	imageBlitInfo.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	imageBlitInfo.srcSubresource.mipLevel = 0;
	imageBlitInfo.srcSubresource.baseArrayLayer = 0;
	imageBlitInfo.srcSubresource.layerCount = 1;

	imageBlitInfo.dstOffsets[0] = { 0, 0, 0 };
	imageBlitInfo.dstOffsets[1] = { (int32_t)dstSize.width, (int32_t)dstSize.height, 1 };
	imageBlitInfo.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	imageBlitInfo.dstSubresource.mipLevel = 0;
	imageBlitInfo.dstSubresource.baseArrayLayer = 0;
	imageBlitInfo.dstSubresource.layerCount = 1;

	vkCmdBlitImage(commandBuffer, srcImage, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, dstImage, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &imageBlitInfo, VK_FILTER_LINEAR);
}

void Vk::Image::Init()
{
	auto device = VulkanContext::GetContext()->GetDevice();

	auto imageInfo = BuildImageInfo();
	VK_CHECK_MESSAGE(vkCreateImage(device->Value(), &imageInfo, nullptr, &image), "Failed to create image!");

	auto allocInfo = Memory::GetImageMemoryAllocInfo(this);
	VK_CHECK_MESSAGE(vkAllocateMemory(device->Value(), &allocInfo, nullptr, &imageMemory), "Failed to allocate image memory!");

	vkBindImageMemory(device->Value(), image, imageMemory, 0);

	for (auto& [name, config] : specification.imageViewConfigs)
	{
		if (config.generateMipImageViewAutomaticly)
		{
			//Default iamge view containing all the mip levels
			imageViews.insert(std::make_pair(name, VK_NULL_HANDLE));
			ImageViewConfig mipConfig = config;
			mipConfig.baseMipLevel = 0;
			mipConfig.mipLevelCount = specification.mipmapLevel;
			auto imageViewInfo = BuildImageViewInfoExtended(mipConfig);
			VK_CHECK_MESSAGE(vkCreateImageView(device->Value(), &imageViewInfo, nullptr, &imageViews.at(name)), "Failed to create texture image view!");

			//Individual mip level image views
			for (uint32_t mipLevel = 0; mipLevel < specification.mipmapLevel; mipLevel++)
			{
				std::string mipName = name + "_mip_" + std::to_string(mipLevel);
				imageViews.insert(std::make_pair(mipName, VK_NULL_HANDLE));
				ImageViewConfig mipConfig = config;
				mipConfig.baseMipLevel = mipLevel;
				mipConfig.mipLevelCount = 1;
				auto imageViewInfo = BuildImageViewInfoExtended(mipConfig);
				VK_CHECK_MESSAGE(vkCreateImageView(device->Value(), &imageViewInfo, nullptr, &imageViews.at(mipName)), "Failed to create texture image view!");
			}
		}
		else
		{
			imageViews.insert(std::make_pair(name, VK_NULL_HANDLE));
			auto imageViewInfo = BuildImageViewInfoExtended(config);
			VK_CHECK_MESSAGE(vkCreateImageView(device->Value(), &imageViewInfo, nullptr, &imageViews.at(name)), "Failed to create texture image view!");
		}
	}
}

void Vk::Image::Destroy()
{
	auto device = VulkanContext::GetContext()->GetDevice();

	for (auto& [name, view] : imageViews)
	{
		if(view != VK_NULL_HANDLE)
			vkDestroyImageView(device->Value(), view, nullptr);
	}
	
	if(image != VK_NULL_HANDLE)
		vkDestroyImage(device->Value(), image, nullptr);
	
	if(imageMemory != VK_NULL_HANDLE)
		vkFreeMemory(device->Value(), imageMemory, nullptr);

	image = VK_NULL_HANDLE;
	imageMemory = VK_NULL_HANDLE;
	imageViews.clear();
}

VkImageCreateInfo Vk::Image::BuildImageInfo()
{
	uint32_t mipLevels = specification.calcualteMipLevelAutomaticly ? GetMipLevels(specification.width, specification.height) : specification.mipmapLevel;

	//Need the automatic mip level to iterate over image views
	specification.mipmapLevel = mipLevels;

	VkImageCreateInfo imageInfo{};
	imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	imageInfo.imageType = specification.type;
	imageInfo.extent.width = specification.width;
	imageInfo.extent.height = specification.height;
	imageInfo.extent.depth = 1;
	imageInfo.mipLevels = specification.mipmapLevel;
	imageInfo.arrayLayers = 1;
	imageInfo.format = specification.format;
	imageInfo.tiling = specification.tiling;
	imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	imageInfo.usage = specification.usage;
	imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
	imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

	return imageInfo;
}

VkImageViewCreateInfo Vk::Image::BuildImageViewInfoExtended(const ImageViewConfig& imageViewConfig)
{
	if (image == VK_NULL_HANDLE)
	{
		throw std::runtime_error("Failed to build image view info extended: Image is null!");
	}

	VkImageViewCreateInfo viewInfo{};
	viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	viewInfo.image = image;
	viewInfo.viewType = imageViewConfig.viewType;
	viewInfo.format = specification.format;
	viewInfo.subresourceRange.aspectMask = specification.aspectFlag;
	viewInfo.subresourceRange.baseMipLevel = imageViewConfig.baseMipLevel;
	viewInfo.subresourceRange.levelCount = imageViewConfig.mipLevelCount;
	viewInfo.subresourceRange.baseArrayLayer = 0;
	viewInfo.subresourceRange.layerCount = 1;
	
	if(imageViewConfig.swizzle.has_value())
		viewInfo.components = imageViewConfig.swizzle.value();

	return viewInfo;
}
