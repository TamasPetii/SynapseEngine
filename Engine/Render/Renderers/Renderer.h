#pragma once
#include "Engine/Vulkan/VulkanContext.h"
#include "Engine/Vulkan/DynamicRendering.h"
#include "Engine/Managers/ResourceManager.h"
#include "Engine/Registry/Registry.h"
#include <optional>
#include <functional>

class Renderer
{
public:
	virtual void Initialize(std::shared_ptr<ResourceManager> resourceManager) = 0;
	virtual void Render(VkCommandBuffer commandBuffer, std::shared_ptr<Registry> registry, std::shared_ptr<ResourceManager> resourceManager, uint32_t frameIndex, std::function<void()> renderFunction = nullptr) = 0;
};

