#pragma once
#include "Engine/Vulkan/VulkanContext.h"
#include "Engine/Vulkan/DynamicRendering.h"
#include "Engine/Managers/ResourceManager.h"
#include "Engine/Registry/Registry.h"
#include <optional>
#include <functional>

class DepthHierarchyBuilder
{
public:
	void BuildDepthHierarchy(VkCommandBuffer commandBuffer, std::shared_ptr<Registry> registry, std::shared_ptr<ResourceManager> resourceManager, uint32_t frameIndex)
private:
	uint32_t GetImageMipLevels(uint32_t width, uint32_t height);
};

