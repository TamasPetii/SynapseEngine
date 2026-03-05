#pragma once
#include "Engine/SynApi.h"
#include "FrameContext.h"
#include <vulkan/vulkan.h>
#include <functional>
#include <span>
#include <memory>

namespace Syn::Vk { 
    class Context;
}

namespace Syn {
    class ResourceManager;
    class StaticMeshBuilder;
    class RenderManager;
}

namespace Syn
{
    struct SYN_API EngineInitParams {
        std::function<void(VkInstance, VkSurfaceKHR*)> createSurfaceCallback;
        std::function<std::pair<uint32_t, uint32_t>()> getWindowExtentCallback;
        std::function<std::vector<const char*>()> getSurfaceExtensionsCallback;
        std::function<void()> onRenderGuiCallback;
    };

	class SYN_API Engine
	{
	public:
        Engine(const EngineInitParams& params);
        ~Engine();

        void Update();
        void Render();
        void WindowResizeEvent(uint32_t width, uint32_t height);
    private:
        void Init(const EngineInitParams& params);
        void InitLogger();
        void InitVulkan(const EngineInitParams& params);
        void InitResourceManager();
        void InitStaticMeshBuilder();
        void InitFrameContext(uint32_t framesInFlight);
        void InitRenderPipelines();
        void Shutdown();
    private:
        void AdvanceFrameIndex();
    private:
		bool _isMinimized = false;
        FrameContext _frameContext;
		std::unique_ptr<Vk::Context> _vkContext;
		std::unique_ptr<ResourceManager> _resourceManager;
        std::unique_ptr<StaticMeshBuilder> _staticMeshBuilder;
		std::unique_ptr<RenderManager> _renderManager;
	};
}


