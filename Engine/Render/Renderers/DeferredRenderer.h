#pragma once
#include "Renderer.h"

class ENGINE_API DeferredRenderer : public Renderer
{
public:
	void Initialize(std::shared_ptr<ResourceManager> resourceManager) override;
	void Render(VkCommandBuffer commandBuffer, std::shared_ptr<Registry> registry, std::shared_ptr<ResourceManager> resourceManager, uint32_t frameIndex, std::function<void()> renderFunction = nullptr) override;
private:
	void RenderEmissiveAo(VkCommandBuffer commandBuffer, std::shared_ptr<Registry> registry, std::shared_ptr<ResourceManager> resourceManager, uint32_t frameIndex);

	void RenderDirectionLights(VkCommandBuffer commandBuffer, std::shared_ptr<Registry> registry, std::shared_ptr<ResourceManager> resourceManager, uint32_t frameIndex);
	void RenderPointLights(VkCommandBuffer commandBuffer, std::shared_ptr<Registry> registry, std::shared_ptr<ResourceManager> resourceManager, uint32_t frameIndex);
	void RenderSpotLights(VkCommandBuffer commandBuffer, std::shared_ptr<Registry> registry, std::shared_ptr<ResourceManager> resourceManager, uint32_t frameIndex);

	void RenderPointLightsIndirect(VkCommandBuffer commandBuffer, std::shared_ptr<Registry> registry, std::shared_ptr<ResourceManager> resourceManager, uint32_t frameIndex);
	void RenderSpotLightsIndirect(VkCommandBuffer commandBuffer, std::shared_ptr<Registry> registry, std::shared_ptr<ResourceManager> resourceManager, uint32_t frameIndex);
};
