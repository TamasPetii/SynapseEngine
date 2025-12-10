#pragma once
#include "Renderer.h"
#include "Engine/Vulkan/VulkanContext.h"
#include "Engine/Managers/ResourceManager.h"
#include "Engine/Registry/Registry.h"
#include <memory>

class BloomRenderer : public Renderer
{
public:
	void Initialize(std::shared_ptr<ResourceManager> resourceManager) override;
	void Render(VkCommandBuffer commandBuffer, std::shared_ptr<Registry> registry, std::shared_ptr<ResourceManager> resourceManager, uint32_t frameIndex, std::function<void()> renderFunction = nullptr) override;
private:
	void Prefilter(VkCommandBuffer cmd, std::shared_ptr<ResourceManager> rm, uint32_t frameIndex);
	void Downsample(VkCommandBuffer cmd, std::shared_ptr<ResourceManager> rm, uint32_t frameIndex);
	void Upsample(VkCommandBuffer cmd, std::shared_ptr<ResourceManager> rm, uint32_t frameIndex);
	void Composite(VkCommandBuffer cmd, std::shared_ptr<ResourceManager> rm, uint32_t frameIndex);
};