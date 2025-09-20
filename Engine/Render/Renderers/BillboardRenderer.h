#pragma once
#include "Engine/Vulkan/VulkanContext.h"
#include "Engine/Vulkan/DynamicRendering.h"
#include "Engine/Managers/ResourceManager.h"
#include "Engine/Registry/Registry.h"

class BillboardRenderer
{
public:
	static void Render(VkCommandBuffer commandBuffer, std::shared_ptr<Registry> registry, std::shared_ptr<ResourceManager> resourceManager, uint32_t frameIndex);
};

