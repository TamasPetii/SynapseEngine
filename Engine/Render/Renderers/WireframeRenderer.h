#pragma once
#include "Renderer.h"

class ENGINE_API WireframeRenderer : public Renderer
{
public:
	void Initialize(std::shared_ptr<ResourceManager> resourceManager) override;
	void Render(VkCommandBuffer commandBuffer, std::shared_ptr<Registry> registry, std::shared_ptr<ResourceManager> resourceManager, uint32_t frameIndex, std::function<void()> renderFunction = nullptr) override;
private:
	void RenderWireframeVolume(VkCommandBuffer commandBuffer, std::shared_ptr<Registry> registry, std::shared_ptr<ResourceManager> resourceManager, uint32_t frameIndex);
	void RenderProjectedColliders(VkCommandBuffer commandBuffer, std::shared_ptr<Registry> registry, std::shared_ptr<ResourceManager> resourceManager, uint32_t frameIndex);
};

