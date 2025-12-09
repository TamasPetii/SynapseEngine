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
	static void Clear(VkCommandBuffer commandBuffer, std::shared_ptr<ResourceManager> resourceManager);
	static void Build(VkCommandBuffer commandBuffer, std::shared_ptr<Registry> registry, std::shared_ptr<ResourceManager> resourceManager, uint32_t frameIndex);
private:
	static void CopyLinearDepthToDepthPyramid(VkCommandBuffer commandBuffer, std::shared_ptr<Registry> registry, std::shared_ptr<ResourceManager> resourceManager, uint32_t frameIndex);
	static void BuildDepthHierarchy(VkCommandBuffer commandBuffer, std::shared_ptr<Registry> registry, std::shared_ptr<ResourceManager> resourceManager, uint32_t frameIndex);
};

