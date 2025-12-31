#pragma once
#include "VkCommon.h"

#include "Core/Instance.h"
#include "Core/PhysicalDevice.h"
#include "Core/Device.h"
#include "Presentation/Surface.h"
#include "Presentation/SwapChain.h"

namespace Syn::Vk {

    struct SYN_API ContextInitParams {
        bool enableValidation = true;

        std::function<std::vector<const char*>()> getSurfaceExtensionsCallback;
        std::function<void(VkInstance, VkSurfaceKHR*)> createSurfaceCallback;
        std::function<VkExtent2D()> getWindowExtentCallback;
    };

    class SYN_API Context {
    public:
        Context(const ContextInitParams& params);
        ~Context();

        auto GetInstance() const { return _instance; }
        auto GetPhysicalDevice() const { return _physicalDevice; }
        auto GetDevice() const { return _device; }
        auto GetSurface() const { return _surface; }
        auto GetSwapChain() const { return _swapChain; }
    private:
        std::shared_ptr<Instance> _instance;
        std::shared_ptr<Surface> _surface;
        std::shared_ptr<PhysicalDevice> _physicalDevice;
        std::shared_ptr<Device> _device;
        std::shared_ptr<SwapChain> _swapChain;
    };
}