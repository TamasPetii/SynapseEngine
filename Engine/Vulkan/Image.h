#pragma once
#include "Engine/EngineApi.h"
#include "VulkanContext.h"
#include <string>
#include <vector>
#include <span>

namespace Vk
{
	class Memory;

	struct ENGINE_API ImageViewConfig
	{
		bool generateMipImageViewAutomaticly = false;
		VkImageViewType viewType;
		uint32_t baseMipLevel = 0;
		uint32_t mipLevelCount = 1;
		uint32_t baseArrayLayer = 0;
		uint32_t layerCount = 1;
		std::optional<VkComponentMapping> swizzle;
	};

	struct ENGINE_API ImageSpecification
	{
		uint32_t width;
		uint32_t height;
		VkImageType type;
		VkFormat format;
		VkImageTiling tiling;
		VkImageUsageFlags usage;
		VkImageAspectFlagBits aspectFlag;
		VkMemoryPropertyFlags memoryProperties;
		uint32_t mipmapLevel = 1;
		uint32_t arrayLayers = 1;
		bool calcualteMipLevelAutomaticly = false;
		VkImageCreateFlags flags = VK_IMAGE_CREATE_FLAG_BITS_MAX_ENUM;

		void AddImageViewConfig(const std::string& name, const ImageViewConfig& imageViewConfig);
		void AddImageViewConfig(const std::string& name, VkImageViewType viewType, uint32_t baseMipLevel = 0, uint32_t mipLevelCount = 1, std::optional<VkComponentMapping> swizzle = std::nullopt, bool calcualteMipLevelAutomaticly = false, uint32_t baseArrayLayer  = 0, uint32_t layerCount = 1);
		std::unordered_map<std::string, ImageViewConfig> imageViewConfigs;
	};

	class ENGINE_API Image
	{
	public:
		Image(const ImageSpecification& specification);
		~Image();
		void Init();
		void Destroy();
		const VkImage Value() const;
		const VkImageView GetImageView(const std::string& imageViewName) const;
		static bool IsDepthFormat(VkFormat format);
		static void CopyImageToImage(VkCommandBuffer commandBuffer, VkImage srcImage, VkExtent2D srcSize, VkImage dstImage, VkExtent2D dstSize);
		static void CopyImageToImageDynamic(VkCommandBuffer commandBuffer, VkImage srcImage, VkExtent2D srcSize, uint32_t srcBaseMipMap, VkImage dstImage, VkExtent2D dstSize, uint32_t dstBaseMipMap);
		static void TransitionImageLayout(VkCommandBuffer commandBuffer, VkImage image, VkImageLayout srcLayout, VkPipelineStageFlags srcStageFlags, VkAccessFlags srcAccessMask, VkImageLayout dstLayout, VkPipelineStageFlags dstStageFlags, VkAccessFlags dstAccessMask, uint32_t mipmapLevel = 1);
		static void TransitionImageLayoutDynamic(VkCommandBuffer commandBuffer, VkImage image, VkImageLayout srcLayout, VkPipelineStageFlags2 srcStageFlags, VkAccessFlags2 srcAccessMask, VkImageLayout dstLayout, VkPipelineStageFlags2 dstStageFlags, VkAccessFlags2 dstAccessMask, uint32_t mipmapLevel = 1, uint32_t baseMipMap = 0);
		static void GenerateMipMaps(VkCommandBuffer commandBuffer, VkImage image, uint32_t width, uint32_t height, uint32_t mipmapLevels);
		static bool ImageFormatSupportsLinearMipMap(VkFormat format);
		static uint32_t GetMipLevels(uint32_t width, uint32_t height);
	private:
		VkImageCreateInfo BuildImageInfo();
		VkImageViewCreateInfo BuildImageViewInfoExtended(const ImageViewConfig& imageViewConfig);
	private:
		VkImage image = VK_NULL_HANDLE;
		VkDeviceMemory imageMemory = VK_NULL_HANDLE;
		std::unordered_map<std::string, VkImageView> imageViews;
		ImageSpecification specification;
		friend class Memory;
	};
}
