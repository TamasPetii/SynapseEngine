#pragma once
#include "Engine/Vulkan/VulkanContext.h"
#include "Engine/Vulkan/Buffer.h"
#include "Engine/Vulkan/CommandBuffer.h"
#include "Engine/Registry/Registry.h"
#include "Engine/Managers/ResourceManager.h"
#include "Renderers/Renderer.h"

#include <array>

class ENGINE_API RenderManager
{
public:
	RenderManager(std::shared_ptr<ResourceManager> resourceManager);
	~RenderManager();
	void Render(std::shared_ptr<Registry> registry, uint32_t frameIndex);
	void SetGuiRenderFunction(const std::function<void(VkCommandBuffer, std::shared_ptr<Registry>, std::shared_ptr<ResourceManager>, uint32_t)>& function);
	void RecreateSwapChain();
private:
	void Init();
	void Destroy();
	void InitRenderers();
	void InitCommandPool();
	void InitCommandBuffer();
private:
	std::shared_ptr<ResourceManager> resourceManager;
	std::array<VkCommandPool, GlobalConfig::FrameConfig::maxFramesInFlights> commandPools;
	std::array<VkCommandBuffer, GlobalConfig::FrameConfig::maxFramesInFlights> commandBuffers;
	std::function<void(VkCommandBuffer, std::shared_ptr<Registry>, std::shared_ptr<ResourceManager>, uint32_t)> guiRenderFunction;	
	std::unordered_map<std::string, std::shared_ptr<Renderer>> renderers;
};

