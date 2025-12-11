#pragma once
#include "Window.h"
#include "Engine/Registry/Registry.h"
#include "IconsFontAwesome5.h"
#include <functional>

class ComponentWindow : public Window
{
public:
	virtual void Render(std::shared_ptr<Registry> registry, std::shared_ptr<ResourceManager> resourceManager, std::set<VkDescriptorSet>& textureSet, uint32_t frameIndex) override;

private:
	void DrawLabel(const char* label);
	void DrawVec3Control(const std::string& label, glm::vec3& values, float resetValue, const std::function<void()>& onUpdate);

	void RenderTagComponent(std::shared_ptr<Registry> registry, Entity entity);
	void RenderTransformComponent(std::shared_ptr<Registry> registry, Entity entity);
	void RenderDirectionLightComponent(std::shared_ptr<Registry> registry, Entity entity);
	void RenderPointLightComponent(std::shared_ptr<Registry> registry, Entity entity);
	void RenderSpotLightComponent(std::shared_ptr<Registry> registry, Entity entity);

	void RenderCameraComponent(std::shared_ptr<Registry> registry, Entity entity);
	void RenderShapeComponent(std::shared_ptr<Registry> registry, Entity entity);
	void RenderModelComponent(std::shared_ptr<Registry> registry, Entity entity);
	void RenderMaterialComponent(std::shared_ptr<Registry> registry, Entity entity);
	void RenderAnimationComponent(std::shared_ptr<Registry> registry, Entity entity);

	void RenderAnimationPanel();
	void RenderAddComponentPopUp(std::shared_ptr<Registry> registry, Entity entity);
};