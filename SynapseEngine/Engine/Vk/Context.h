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

        Instance& GetInstance() const { return *_instance; }
        PhysicalDevice& GetPhysicalDevice() const { return *_physicalDevice; }
        Device& GetDevice() const { return *_device; }
        Surface& GetSurface() const { return *_surface; }
        SwapChain& GetSwapChain() const { return *_swapChain; }
    private:
        std::unique_ptr<Instance> _instance;
        std::unique_ptr<Surface> _surface;
        std::unique_ptr<PhysicalDevice> _physicalDevice;
        std::unique_ptr<Device> _device;
        std::unique_ptr<SwapChain> _swapChain;
    };
}