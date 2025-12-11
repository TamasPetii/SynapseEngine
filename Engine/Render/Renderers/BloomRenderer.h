#pragma once
#include "Renderer.h"
#include "Engine/Vulkan/VulkanContext.h"
#include "Engine/Managers/ResourceManager.h"
#include "Engine/Registry/Registry.h"
#include <memory>

struct ENGINE_API BloomSettings
{
	bool enabled = false;
	float threshold = 1.0f;
	float knee = 0.1f;
	float upsampleRadius = 0.005f;
	float bloomStrength = 0.04f;
	float exposure = 1.0f;
};

class ENGINE_API BloomRenderer : public Renderer
{
public:
	static BloomSettings settings;

	void Initialize(std::shared_ptr<ResourceManager> resourceManager) override;
	void Render(VkCommandBuffer commandBuffer, std::shared_ptr<Registry> registry, std::shared_ptr<ResourceManager> resourceManager, uint32_t frameIndex, std::function<void()> renderFunction = nullptr) override;
private:
	void Prefilter(VkCommandBuffer cmd, std::shared_ptr<ResourceManager> rm, uint32_t frameIndex);
	void Downsample(VkCommandBuffer cmd, std::shared_ptr<ResourceManager> rm, uint32_t frameIndex);
	void Upsample(VkCommandBuffer cmd, std::shared_ptr<ResourceManager> rm, uint32_t frameIndex);
	void Composite(VkCommandBuffer cmd, std::shared_ptr<ResourceManager> rm, uint32_t frameIndex);
};