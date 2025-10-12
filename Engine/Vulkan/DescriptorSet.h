#pragma once
#include "../EngineApi.h"
#include "VulkanContext.h"
#include <unordered_map>
#include <span>

namespace Vk
{
	struct ENGINE_API DescriptorLayoutConfig
	{
		uint32_t binding;
		VkDescriptorType type;
		VkShaderStageFlags flags;
		uint32_t descriptorCount = 1;
	};

	struct ENGINE_API DescriptorLayoutBuffer
	{
		VkBuffer buffer;
		VkDeviceSize offset;
		VkDeviceSize range;
		DescriptorLayoutConfig config;
	};

	struct ENGINE_API DescriptorLayoutImage
	{
		VkImageView image;
		VkSampler sampler;
		VkImageLayout layout;
		DescriptorLayoutConfig config;
	};

	class ENGINE_API DescriptorSet
	{
	public:
		DescriptorSet(VkDescriptorPool pool, const std::unordered_map<std::string, DescriptorLayoutBuffer>& bufferLayouts, const std::unordered_map<std::string, DescriptorLayoutImage>& imageLayouts);
		~DescriptorSet();
		void Free();
		const VkDescriptorSet& Value() const;
		const VkDescriptorSetLayout& Layout() const;
		void UpdateImageArrayElement(const std::string& name, VkImageView imageView, VkSampler sampler, uint32_t index);
	private:
		void Initialize();
		void Cleanup();
	private:
		VkDescriptorPool pool = VK_NULL_HANDLE;
		VkDescriptorSet descriptorSet = VK_NULL_HANDLE;
		VkDescriptorSetLayout descriptorlayout = VK_NULL_HANDLE;

		std::unordered_map<std::string, DescriptorLayoutBuffer> bufferLayouts;
		std::unordered_map<std::string, DescriptorLayoutImage> imageLayouts;
	};

	class ENGINE_API PushDescriptorSet
	{
	public:
		PushDescriptorSet(const std::unordered_map<std::string, DescriptorLayoutBuffer>& bufferLayouts, const std::unordered_map<std::string, DescriptorLayoutImage>& imageLayouts);
		~PushDescriptorSet();
		const VkDescriptorSetLayout& Layout() const;
		const DescriptorLayoutBuffer& GetBufferLayout(const std::string& name) const { return bufferLayouts.at(name); }
		const DescriptorLayoutImage& GetImageLayout(const std::string& name) const { return imageLayouts.at(name); }
	private:
		VkDescriptorSetLayout descriptorlayout = VK_NULL_HANDLE;
		std::unordered_map<std::string, DescriptorLayoutBuffer> bufferLayouts;
		std::unordered_map<std::string, DescriptorLayoutImage> imageLayouts;
	};

	class ENGINE_API DescriptorSetBuilder
	{
	public:
		DescriptorSetBuilder& AddDescriptorLayoutBuffer(const std::string& name, uint32_t binding, VkDescriptorType type, VkShaderStageFlags flags, VkBuffer buffer, VkDeviceSize offset, VkDeviceSize range);
		DescriptorSetBuilder& AddDescriptorLayoutImage(const std::string& name, uint32_t binding, VkDescriptorType type, VkShaderStageFlags flags, VkImageView image, VkSampler sampler, VkImageLayout layout, uint32_t descriptorCount = 1);
		
		std::shared_ptr<DescriptorSet> BuildDescriptorSet(VkDescriptorPool pool);
		std::shared_ptr<PushDescriptorSet> BuildPushDescriptorSet();
	private:
		std::unordered_map<std::string, DescriptorLayoutBuffer> bufferLayouts;
		std::unordered_map<std::string, DescriptorLayoutImage> imageLayouts;
	};

	class ENGINE_API DesciptorSetUtils
	{
	public:
		static VkDescriptorSetLayoutBinding BuildLayoutBindingInfo(uint32_t binding, VkDescriptorType type, VkShaderStageFlags flags, uint32_t descriptorCount);
		static VkWriteDescriptorSet BuildWritePushDescriptorSetInfo(const DescriptorLayoutImage& imageLayout, const VkDescriptorImageInfo& imageInfo);
	};
}