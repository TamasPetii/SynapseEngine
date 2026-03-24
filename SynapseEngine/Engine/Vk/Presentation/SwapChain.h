#pragma once
#include "../VkCommon.h"
#include "../Core/Device.h"
#include "../Core/PhysicalDevice.h"
#include "Surface.h"

#include "Engine/Vk/Image/Image.h"

namespace Syn::Vk {

    struct SYN_API SwapChainSupportDetails {
        VkSurfaceCapabilitiesKHR capabilities;
        std::vector<VkSurfaceFormatKHR> formats;
        std::vector<VkPresentModeKHR> presentModes;
    };

    class SYN_API SwapChain {
    public:
        SwapChain(const PhysicalDevice& physicalDevice, const Device& device, const Surface& surface, std::function<VkExtent2D()> getWindowExtentCallback);
        ~SwapChain();

        SwapChain(const SwapChain&) = delete;
        SwapChain& operator=(const SwapChain&) = delete;

        SwapChain(SwapChain&&) noexcept = default;
        SwapChain& operator=(SwapChain&&) noexcept = default;

        void Recreate();

        VkSwapchainKHR Handle() const { return _handle; }
        VkFormat GetImageFormat() const { return _imageFormat; }
        VkExtent2D GetExtent() const { return _extent; }
        uint32_t GetImageCount() const { return static_cast<uint32_t>(_images.size()); }

		Image* GetImage(uint32_t index) const { return _images[index].get(); }

        uint32_t AcquireNextImage(VkSemaphore presentSemaphore);
        void Present(uint32_t imageIndex, VkSemaphore renderFinishedSemaphore, VkFence presentFence);
    private:
        void Init();
        void Cleanup();

        SwapChainSupportDetails QuerySwapChainSupport();
        VkSurfaceFormatKHR ChooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);
        VkPresentModeKHR ChooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes);
        VkExtent2D ChooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities);
    private:
        const PhysicalDevice& _physicalDevice;
        const Device& _device;
        const Surface& _surface;

        VkSwapchainKHR _handle = VK_NULL_HANDLE;
        VkFormat _imageFormat;
        VkExtent2D _extent;
        std::vector<std::unique_ptr<Image>> _images;
        std::function<VkExtent2D()> _getWindowExtentCallback;
    };
}