#pragma once
#include "Engine/SynApi.h"
#include "FrameContext.h"
#include <vulkan/vulkan.h>
#include <functional>
#include <span>
#include <memory>

#include "Profiler/TaskProfilerObserver.h"
#include <taskflow/taskflow.hpp>

namespace Syn::Vk { 
    class Context;
    class GpuUploader;
}

namespace Syn {
    class ResourceManager;
    class StaticMeshBuilder;
    class RenderManager;
    class InputManager;
    class SceneManager;
}

namespace Syn
{
    struct SYN_API EngineInitParams {
        std::function<void(VkInstance, VkSurfaceKHR*)> createSurfaceCallback;
        std::function<std::pair<uint32_t, uint32_t>()> getWindowExtentCallback;
        std::function<std::vector<const char*>()> getSurfaceExtensionsCallback;
        std::function<void(VkCommandBuffer)> onRenderGuiCallback;
    };

	class SYN_API Engine
	{
	public:
        Engine(const EngineInitParams& params);
        ~Engine();

        Vk::Context* GetVkContext() { return _vkContext.get(); }
        SceneManager* GetSceneManager() { return _sceneManager.get(); }

        void Update(float deltaTime);
        void Render();
        void WindowResizeEvent(uint32_t width, uint32_t height);

        void OnKey(int key, int scancode, int action, int mods);
        void OnMouseButton(int button, int action, int mods);
        void OnMouseMove(float x, float y);
        void SetInputEnabled(bool enabled) { _inputEnabled = enabled; }
    private:
        void Init(const EngineInitParams& params);
        void InitLogger();
        void InitVulkan(const EngineInitParams& params);
        void InitResourceManager();
        void InitFrameContext(uint32_t framesInFlight);
        void InitRenderManager();
        void InitTaskExecutor();
        void InitSceneManager();
        void Shutdown();
    private:
        void AdvanceFrameIndex();
    private:
		bool _isMinimized = false;
        bool _inputEnabled = true;
        FrameContext _frameContext;
		std::unique_ptr<Vk::Context> _vkContext;
        std::unique_ptr<Vk::GpuUploader> _gpuUploader;
		std::unique_ptr<ResourceManager> _resourceManager;
		std::unique_ptr<RenderManager> _renderManager;
        std::unique_ptr<InputManager> _inputManager;
        std::unique_ptr<SceneManager> _sceneManager;
		std::unique_ptr<tf::Executor> _taskExecutor;
        std::shared_ptr<tf::TFProfObserver> _jsonTaskObserver;
        std::shared_ptr<TaskProfilerObserver> _guiTaskObserver;
	};
}


