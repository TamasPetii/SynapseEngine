#pragma once
#include "Renderer.h"

class BillboardRenderer : public Renderer
{
public:
	void Initialize(std::shared_ptr<ResourceManager> resourceManager) override;
	void Render(VkCommandBuffer commandBuffer, std::shared_ptr<Registry> registry, std::shared_ptr<ResourceManager> resourceManager, uint32_t frameIndex, std::function<void()> renderFunction = nullptr) override;
private:
	std::shared_ptr<ImageTexture> directionLightIcon = nullptr;
	std::shared_ptr<ImageTexture> pointLightIcon = nullptr;
	std::shared_ptr<ImageTexture> spotLightIcon = nullptr;
};

