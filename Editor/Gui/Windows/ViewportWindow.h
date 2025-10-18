#pragma once
#include "Window.h"
#include "Editor/Gui/ImGui/ImGuizmo.h"

class ViewportWindow : public Window
{
public:
	virtual void Render(std::shared_ptr<Registry> registry, std::shared_ptr<ResourceManager> resourceManager, std::set<VkDescriptorSet>& textureSet, uint32_t frameIndex) override;
protected:
	struct GizmoConfig
	{
		ImGuizmo::OPERATION operation = ImGuizmo::TRANSLATE;
		ImGuizmo::MODE mode = ImGuizmo::WORLD;
		bool useSnap = false;
		float snapTranslate[3] = { 1.0f, 1.0f, 1.0f };
		float snapAngle = 5.0f;
		float snapScale = 0.1f;
	};
	inline static int viewportImage = 0;
	inline static GizmoConfig gizmoConfig;
private:
	std::pair<std::string, std::string> GetViewportImageName();
	void GizmoConfigMenu();
	void ViewportImageMenu();
	void RenderGizmo(std::shared_ptr<Registry> registry, ImVec2 viewPortStart, ImVec2 viewPortEnd);
	void GetClickedActiveEntity(std::shared_ptr<Registry> registry, std::shared_ptr<Vk::FrameBuffer> frameBuffer);
};

