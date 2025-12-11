#pragma once
#include "Window.h"
#include <vector>
#include <string>

struct TagComponent;

class EntityWindow : public Window
{
public:
	virtual void Render(std::shared_ptr<Registry> registry, std::shared_ptr<ResourceManager> resourceManager, std::set<VkDescriptorSet>& textureSet, uint32_t frameIndex) override;

private:
	void DrawEntityNode(std::shared_ptr<Registry> registry, Entity entity);
	void RenderContextMenu(std::shared_ptr<Registry> registry, std::shared_ptr<ResourceManager> resourceManager);
	std::string GetEntityIcon(std::shared_ptr<Registry> registry, Entity entity);
private:
	std::vector<Entity> m_EntitiesToDelete;
	std::vector<Entity> m_EntitiesToDuplicate;
};