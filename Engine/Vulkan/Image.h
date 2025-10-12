#pragma once
#include "Engine/EngineApi.h"
#include "VulkanContext.h"
#include <string>
#include <vector>
#include <span>

namespace Vk
{
	class Memory;

	struct ENGINE_API ImageSpecification
	{
		uint32_t width;
		uint32_t height;
		VkImageType type;
		VkFormat format;
		VkImageTiling tiling;
		VkImageUsageFlags usage;
		VkImageViewType viewType;
		VkImageAspectFlagBits aspectFlag;
		VkMemoryPropertyFlags memoryProperties;
		uint32_t mipmapLevel = 1;
		bool calcualteMipLevelAutomatic = false;
		std::unordered_map<std::string, VkComponentMapping> swizzles;
	};

	class ENGINE_API Image
	{
	public:
		Image(const ImageSpecification& specification);
		~Image();
		void Init();
		void Destroy();
		const VkImage Value() const;
		const VkImageView& GetImageView() const;
		static bool IsDepthFormat(VkFormat format);
		static void CopyImageToImage(VkCommandBuffer commandBuffer, VkImage srcImage, VkExtent2D srcSize, VkImage dstImage, VkExtent2D dstSize);
		static void CopyImageToImageDynamic(VkCommandBuffer commandBuffer, VkImage srcImage, VkExtent2D srcSize, uint32_t srcBaseMipMap, VkImage dstImage, VkExtent2D dstSize, uint32_t dstBaseMipMap);
		static void TransitionImageLayout(VkCommandBuffer commandBuffer, VkImage image, VkImageLayout srcLayout, VkPipelineStageFlags srcStageFlags, VkAccessFlags srcAccessMask, VkImageLayout dstLayout, VkPipelineStageFlags dstStageFlags, VkAccessFlags dstAccessMask, uint32_t mipmapLevel = 1);
		static void TransitionImageLayoutDynamic(VkCommandBuffer commandBuffer, VkImage image, VkImageLayout srcLayout, VkPipelineStageFlags2 srcStageFlags, VkAccessFlags2 srcAccessMask, VkImageLayout dstLayout, VkPipelineStageFlags2 dstStageFlags, VkAccessFlags2 dstAccessMask, uint32_t mipmapLevel = 1, uint32_t baseMipMap = 0);
		static void GenerateMipMaps(VkCommandBuffer commandBuffer, VkImage image, uint32_t width, uint32_t height, uint32_t mipmapLevels);
		static bool ImageFormatSupportsLinearMipMap(VkFormat format);
		static uint32_t GetImageMipLevels(uint32_t width, uint32_t height);
	private:
		VkImageCreateInfo BuildImageInfo();
		VkImageViewCreateInfo BuildImageViewInfo();
		VkImageViewCreateInfo BuildImageViewSwizzledInfo(const VkComponentMapping& swizzle);
	private:
		VkImage image = VK_NULL_HANDLE;
		VkImageView imageView = VK_NULL_HANDLE;
		VkDeviceMemory imageMemory = VK_NULL_HANDLE;
		ImageSpecification specification;

		std::unordered_map<std::string, VkImageView> swizzledImageViews;

		friend class Memory;
	};
}
