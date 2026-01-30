#pragma once
#include "Engine/SynApi.h"
#include <vulkan/vulkan.h>
#include <functional>
#include <span>
#include <memory>

namespace Syn::Vk { 
    class Context;
}

namespace Syn {
    class ResourceManager;
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
        void Shutdown();
    private:
		bool _isMinimized = false;
		std::unique_ptr<Vk::Context> _vkContext;
		std::unique_ptr<ResourceManager> _resourceManager;
	};
}


