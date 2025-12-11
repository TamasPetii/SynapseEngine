#pragma once
#include "Window.h"

class GlobalSettingsWindow : public Window
{
public:
	virtual void Render(std::shared_ptr<Registry> registry, std::shared_ptr<ResourceManager> resourceManager, std::set<VkDescriptorSet>& textureSet, uint32_t frameIndex) override;
private:
	void RenderWireframeSettings();
	void RenderBloomSettings();
};

