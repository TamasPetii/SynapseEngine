#include "ImageSampler.h"

Vk::ImageSampler::ImageSampler(const ImageSamplerConfig& config) : 
	config(config)
{
	Init();
}

Vk::ImageSampler::~ImageSampler()
{
	Destroy();
}

const VkSampler Vk::ImageSampler::Value() const
{
	return sampler;
}

void Vk::ImageSampler::Init()
{
	auto device = VulkanContext::GetContext()->GetDevice();
	auto physicalDevice = VulkanContext::GetContext()->GetPhysicalDevice();

	VkSamplerCreateInfo samplerInfo{};
	samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
	samplerInfo.magFilter = config.magFilter;
	samplerInfo.minFilter = config.minFilter;
	samplerInfo.addressModeU = config.addressMode;
	samplerInfo.addressModeV = config.addressMode;
	samplerInfo.addressModeW = config.addressMode;

	if (config.anisotropyEnable)
	{
		VkPhysicalDeviceProperties properties{};
		vkGetPhysicalDeviceProperties(physicalDevice->Value(), &properties);
		samplerInfo.anisotropyEnable = VK_TRUE;
		samplerInfo.maxAnisotropy = properties.limits.maxSamplerAnisotropy;
	}

	samplerInfo.borderColor = config.borderColor;
	samplerInfo.unnormalizedCoordinates = VK_FALSE;
	samplerInfo.compareEnable = VK_FALSE;
	samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
	samplerInfo.mipmapMode = config.mipMapFilter;
	samplerInfo.mipLodBias = 0.0f;
	samplerInfo.minLod = config.minLod;
	samplerInfo.maxLod = config.maxLod;

	VkSamplerReductionModeCreateInfoEXT createInfoReduction = {};
	if (config.reductionMode.has_value())
	{
		createInfoReduction.sType = VK_STRUCTURE_TYPE_SAMPLER_REDUCTION_MODE_CREATE_INFO_EXT;
		createInfoReduction.reductionMode = config.reductionMode.value();
		samplerInfo.pNext = &createInfoReduction;
	}

	VK_CHECK_MESSAGE(vkCreateSampler(device->Value(), &samplerInfo, nullptr, &sampler), "Failed to create texture sampler!");
}

void Vk::ImageSampler::Destroy()
{
	auto device = VulkanContext::GetContext()->GetDevice();

	if (sampler != VK_NULL_HANDLE)
	{
		vkDestroySampler(device->Value(), sampler, nullptr);
		sampler = VK_NULL_HANDLE;
	}
}
