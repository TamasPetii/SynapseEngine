#pragma once
#include "Engine/SynApi.h"
#include "FrameContext.h"
#include <vulkan/vulkan.h>
#include <functional>
#include <span>
#include <memory>

#include <chrono>
#include <taskflow/taskflow.hpp>
#include "Logger/Sink/MemorySink.h"

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
    class IPhysicsEngine;
	class IGpuProfiler;
    class ICpuProfiler;
    class Serializer;
    class MaterialManager;
    class ImageManager;
    class ModelManager;
    class AnimationManager;
    class IRenderStatCollector;
    class FrameStatisticsManager;
}

namespace Syn
{
    struct SYN_API EngineInitParams {
        std::function<void(VkInstance, VkSurfaceKHR*)> createSurfaceCallback;
        std::function<std::pair<uint32_t, uint32_t>()> getWindowExtentCallback;
        std::function<std::vector<const char*>()> getSurfaceExtensionsCallback;
        std::function<void(VkCommandBuffer)> onRenderGuiCallback;
        std::function<void(uint32_t)> onGuiFlushCallback;
    };

	class SYN_API Engine
	{
	public:
        Engine(const EngineInitParams& params);
        ~Engine();

        Vk::Context* GetVkContext() { return _vkContext.get(); }
        SceneManager* GetSceneManager() { return _sceneManager.get(); }
        RenderManager* GetRenderManager() { return _renderManager.get(); }

        void Update(float deltaTime);
        void Render();
        void WindowResizeEvent(uint32_t width, uint32_t height);

        void OnKey(int key, int scancode, int action, int mods);
        void OnMouseButton(int button, int action, int mods);
        void OnMouseMove(float x, float y);
		void OnScroll(float xOffset, float yOffset);
        void SetInputEnabled(bool enabled) { _inputEnabled = enabled; }
        void OnChar(unsigned int codepoint);
    public:
        MaterialManager* GetMaterialManager();
        ImageManager* GetImageManager();
        ModelManager* GetModelManager();
        AnimationManager* GetAnimationManager();
        std::shared_ptr<Syn::MemorySink> GetMemorySink() const { return _memorySink; }
    private:
        void Init(const EngineInitParams& params);
        void InitLogger();
        void InitVulkan(const EngineInitParams& params);
        void InitResourceManager();
        void InitFrameContext(uint32_t framesInFlight);
        void InitRenderManager(const EngineInitParams& params);
        void InitTaskExecutor();
        void InitSceneManager();
        void InitPhysicsEngine();
        void InitProfilers();
        void InitSerializer();
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
        std::unique_ptr<IGpuProfiler> _gpuProfiler;
        std::unique_ptr<ICpuProfiler> _cpuProfiler;
		std::unique_ptr<Serializer> _serializer;
        std::shared_ptr<MemorySink> _memorySink;
		std::unique_ptr<IRenderStatCollector> _renderStatCollector;
		std::unique_ptr<FrameStatisticsManager> _frameStatisticsManager;

        std::function<void(uint32_t)> _onGuiFlushCallback;
    };
}


