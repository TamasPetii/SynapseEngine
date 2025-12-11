#pragma once
#include "Window.h"
#include <set>
#include <string>
#include <vector>

class BenchmarkWindow : public Window
{
public:
	virtual void Render(std::shared_ptr<Registry> registry, std::shared_ptr<ResourceManager> resourceManager, std::set<VkDescriptorSet>& textureSet, uint32_t frameIndex) override;

private:
	void RenderSystemTimes(std::shared_ptr<ResourceManager> resourceManager);
	void RenderGpuDebug(std::shared_ptr<ResourceManager> resourceManager, uint32_t frameIndex);
};