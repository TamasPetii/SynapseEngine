#pragma once
#include "Renderer.h"

class ObjectCuller : public Renderer
{
public:
	void Initialize(std::shared_ptr<ResourceManager> resourceManager) override;
	void Render(VkCommandBuffer commandBuffer, std::shared_ptr<Registry> registry, std::shared_ptr<ResourceManager> resourceManager, uint32_t frameIndex, std::function<void()> renderFunction = nullptr) override;
private:
	void CullObjectInCameraFrustum(VkCommandBuffer commandBuffer, std::shared_ptr<Registry> registry, std::shared_ptr<ResourceManager> resourceManager, uint32_t frameIndex);
	void CullPointLightInCameraFrustum(VkCommandBuffer commandBuffer, std::shared_ptr<Registry> registry, std::shared_ptr<ResourceManager> resourceManager, uint32_t frameIndex);
};

