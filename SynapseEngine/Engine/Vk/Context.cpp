#define VOLK_IMPLEMENTATION
#include "Context.h"

namespace Syn::Vk {

    static std::vector<const char*> GetRequiredExtensions() {
        return {
            VK_KHR_SWAPCHAIN_EXTENSION_NAME,
            VK_KHR_PUSH_DESCRIPTOR_EXTENSION_NAME,
            VK_EXT_MESH_SHADER_EXTENSION_NAME,
            VK_EXT_SHADER_OBJECT_EXTENSION_NAME,
            VK_EXT_DESCRIPTOR_BUFFER_EXTENSION_NAME,
            VK_EXT_EXTENDED_DYNAMIC_STATE_EXTENSION_NAME,
            VK_EXT_EXTENDED_DYNAMIC_STATE_2_EXTENSION_NAME,
            VK_EXT_EXTENDED_DYNAMIC_STATE_3_EXTENSION_NAME,
            VK_EXT_MEMORY_BUDGET_EXTENSION_NAME,
            VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME,
            VK_KHR_SYNCHRONIZATION_2_EXTENSION_NAME
        };
    }

    Context::Context(const ContextInitParams& params)
    {
        std::vector<const char*> instanceExts;
        if (params.getSurfaceExtensionsCallback) {
            instanceExts = params.getSurfaceExtensionsCallback();
        }
        _instance = std::make_unique<Instance>(params.enableValidation, instanceExts);

        SYN_ASSERT(params.createSurfaceCallback, "Surface creation callback is missing!");

        _surface = std::make_unique<Surface>(*_instance, params.createSurfaceCallback);

        auto extensions = GetRequiredExtensions();
        _physicalDevice = std::make_unique<PhysicalDevice>(_instance->Handle(), _surface->Handle(), extensions);
        _device = std::make_unique<Device>(_instance->Handle(), *_physicalDevice, extensions);
    }

    void Context::InitSwapChain(const ContextInitParams& params)
    {
        SYN_ASSERT(params.getWindowExtentCallback, "Window extent callback is missing!");
        _swapChain = std::make_unique<SwapChain>(*_physicalDevice, *_device, *_surface, params.getWindowExtentCallback);
    }

    Context::~Context() {
    }
}