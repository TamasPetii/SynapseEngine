#pragma once
#include "../EngineApi.h"
#include "VulkanContext.h"

namespace Vk
{
	constexpr uint32_t MAX_SAMPLER_MIMAP_LEVEL = 12;

	struct ENGINE_API ImageSamplerConfig
	{
		VkFilter magFilter;
		VkFilter minFilter;
		VkSamplerMipmapMode mipMapFilter;
		VkSamplerAddressMode addressMode;
		VkBool32 anisotropyEnable;
		VkBorderColor borderColor;
		uint32_t minLod = 0.f;
		uint32_t maxLod = 0.f;
		std::optional<VkSamplerReductionMode> reductionMode = std::nullopt;
	};

	class ENGINE_API ImageSampler
	{
	public:
		ImageSampler(const ImageSamplerConfig& specification);
		~ImageSampler();
		const VkSampler Value() const;
	private:
		void Init();
		void Destroy();
		ImageSamplerConfig config;
		VkSampler sampler = VK_NULL_HANDLE;
	};
}
