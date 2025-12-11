#pragma once
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_ENABLE_EXPERIMENTAL

#include "EngineApi.h"
#include <span>
#include <functional>

#include "Engine/Timer/Timer.h"
#include "Engine/Scene/Scene.h"
#include "Engine/Render/RenderManager.h"
#include "Engine/Managers/ResourceManager.h"
#include "Engine/Managers/InputManager.h"

struct ENGINE_API VisibleInstanceData {
	std::string name;
	uint32_t count = 0;
	std::vector<uint32_t> instanceIndices;
};

struct ENGINE_API LightShadowStats {
	uint32_t shadowIndex;
	std::vector<VisibleInstanceData> visibleModels;
	std::vector<VisibleInstanceData> visibleShapes;
};

struct ENGINE_API CullingFrameStats {
	uint32_t mainModelCount = 0;
	uint32_t mainShapeCount = 0;

	uint32_t pointLightCount = 0;
	uint32_t pointShadowCount = 0;
	uint32_t pointObjectCount = 0;
	uint32_t pointDispatchX = 0;
	uint32_t pointDispatchY = 0;
	std::vector<LightShadowStats> pointShadowDetails;

	uint32_t spotLightCount = 0;
	uint32_t spotShadowCount = 0;
	uint32_t spotObjectCount = 0;
	uint32_t spotDispatchX = 0;
	uint32_t spotDispatchY = 0;
	std::vector<LightShadowStats> spotShadowDetails;
};

class ENGINE_API Engine
{
public:
	Engine();
	~Engine();
	void Initialize();
	void SimulateFrame();
	void WindowResizeEvent();
	void SetRequiredWindowExtensions(std::span<const char*> extensionNames);
	void SetSurfaceCreationFunction(const std::function<void(const Vk::Instance* const, VkSurfaceKHR* surface)>& function);
	void SetWindowExtentFunction(const std::function<std::pair<int, int>()>& function);
	void SetGuiRenderFunction(const std::function<void(VkCommandBuffer, std::shared_ptr<Registry>, std::shared_ptr<ResourceManager>, uint32_t)>& function);
	static CullingFrameStats stats;
private:
	void Cleanup();
	void Render();
	void Update();
	void UpdateGPU();
	void Finish();
	void CollectCullingStats(uint32_t frameIndex);
private:
	uint32_t frameIndex = 0;
	std::shared_ptr<Scene> scene;
	std::shared_ptr<Timer> frameTimer;
	std::shared_ptr<RenderManager> renderManager;
	std::shared_ptr<ResourceManager> resourceManager;
};