#pragma once
#include "Engine/SynApi.h"
#include <vulkan/vulkan.h>
#include <functional>
#include <span>
#include <memory>

namespace Syn::Vk { class Context; }

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

        void SimulateFrame() {}
        void WindowResizeEvent() {}
    private:
        void Init(const EngineInitParams& params);
        void Shutdown();
    private:
		std::unique_ptr<Vk::Context> _vkContext;
	};
}


