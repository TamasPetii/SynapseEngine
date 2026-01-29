#include "SwapChain.h"
#include <algorithm>
#include <limits>

namespace Syn::Vk {

    SwapChain::SwapChain(const PhysicalDevice& physicalDevice, const Device& device, const Surface& surface, std::function<VkExtent2D()> getWindowExtentCallback)
        : _physicalDevice(physicalDevice), _device(device), _surface(surface), _getWindowExtentCallback(getWindowExtentCallback)
    {
        Init();
    }

    SwapChain::~SwapChain() {
        Cleanup();
    }

    void SwapChain::Recreate() {

		VkExtent2D extent = _getWindowExtentCallback();
 
        if (extent.width == 0 || extent.height == 0) {
            Cleanup();
            return;
        }

        vkDeviceWaitIdle(_device.Handle());
        Cleanup();
        Init();
    }

    void SwapChain::Init() {
        SwapChainSupportDetails swapChainSupport = QuerySwapChainSupport();

        if (swapChainSupport.capabilities.currentExtent.width == 0 && swapChainSupport.capabilities.currentExtent.height == 0) {
            return;
        }

        VkSurfaceFormatKHR surfaceFormat = ChooseSwapSurfaceFormat(swapChainSupport.formats);
        VkPresentModeKHR presentMode = ChooseSwapPresentMode(swapChainSupport.presentModes);
        VkExtent2D extent = ChooseSwapExtent(swapChainSupport.capabilities);

        if (extent.width == 0 || extent.height == 0) return;

        uint32_t imageCount = swapChainSupport.capabilities.minImageCount + 1;
        if (swapChainSupport.capabilities.maxImageCount > 0 && imageCount > swapChainSupport.capabilities.maxImageCount) {
            imageCount = swapChainSupport.capabilities.maxImageCount;
        }

        VkSwapchainCreateInfoKHR createInfo{ VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR };
        createInfo.surface = _surface.Handle();
        createInfo.minImageCount = imageCount;
        createInfo.imageFormat = surfaceFormat.format;
        createInfo.imageColorSpace = surfaceFormat.colorSpace;
        createInfo.imageExtent = extent;
        createInfo.imageArrayLayers = 1;
        createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;

        QueueFamilyIndices indices = _physicalDevice.GetQueueFamilies();
        uint32_t queueFamilyIndices[] = { indices.graphics.value(), indices.present.value() };

        if (indices.graphics != indices.present) {
            createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
            createInfo.queueFamilyIndexCount = 2;
            createInfo.pQueueFamilyIndices = queueFamilyIndices;
        }
        else {
            createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
        }

        createInfo.preTransform = swapChainSupport.capabilities.currentTransform;
        createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
        createInfo.presentMode = presentMode;
        createInfo.clipped = VK_TRUE;
        createInfo.oldSwapchain = VK_NULL_HANDLE;

        SYN_VK_ASSERT_MSG(vkCreateSwapchainKHR(_device.Handle(), &createInfo, nullptr, &_handle), "Failed to create SwapChain");

        vkGetSwapchainImagesKHR(_device.Handle(), _handle, &imageCount, nullptr);
        _images.resize(imageCount);
        vkGetSwapchainImagesKHR(_device.Handle(), _handle, &imageCount, _images.data());

        _imageFormat = surfaceFormat.format;
        _extent = extent;

        _imageViews.resize(_images.size());
        for (size_t i = 0; i < _images.size(); i++) {
            VkImageViewCreateInfo viewInfo{ VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO };
            viewInfo.image = _images[i];
            viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
            viewInfo.format = _imageFormat;
            viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            viewInfo.subresourceRange.baseMipLevel = 0;
            viewInfo.subresourceRange.levelCount = 1;
            viewInfo.subresourceRange.baseArrayLayer = 0;
            viewInfo.subresourceRange.layerCount = 1;

            SYN_VK_ASSERT_MSG(vkCreateImageView(_device.Handle(), &viewInfo, nullptr, &_imageViews[i]), "Failed to create SwapChain Image View");
        }
    }

    void SwapChain::Cleanup() {
        for (auto imageView : _imageViews) {
            if (imageView != VK_NULL_HANDLE)
                vkDestroyImageView(_device.Handle(), imageView, nullptr);
        }
        _imageViews.clear();

        if (_handle != VK_NULL_HANDLE) {
            vkDestroySwapchainKHR(_device.Handle(), _handle, nullptr);
            _handle = VK_NULL_HANDLE;
        }
        _images.clear();
    }

    uint32_t SwapChain::AcquireNextImage(VkSemaphore presentSemaphore) {
        if (_handle == VK_NULL_HANDLE) {
            return -1;
        }

        uint32_t imageIndex;
        VkResult result = vkAcquireNextImageKHR(_device.Handle(), _handle, UINT64_MAX, presentSemaphore, VK_NULL_HANDLE, &imageIndex);

        if (result == VK_ERROR_OUT_OF_DATE_KHR) {
            Recreate();
            return -1;
        }
        else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
            SYN_VK_ASSERT_MSG(result, "Failed to acquire swapchain image");
        }
        return imageIndex;
    }

    void SwapChain::Present(uint32_t imageIndex, VkSemaphore renderFinishedSemaphore) {
        if (_handle == VK_NULL_HANDLE) return;

        VkPresentInfoKHR presentInfo{ VK_STRUCTURE_TYPE_PRESENT_INFO_KHR };
        presentInfo.waitSemaphoreCount = 1;
        presentInfo.pWaitSemaphores = &renderFinishedSemaphore;

        VkSwapchainKHR swapChains[] = { _handle };
        presentInfo.swapchainCount = 1;
        presentInfo.pSwapchains = swapChains;
        presentInfo.pImageIndices = &imageIndex;

        VkQueue presentQueue = _device.GetGraphicsQueue()->Handle();
        VkResult result = vkQueuePresentKHR(presentQueue, &presentInfo);

        if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR) {
            Recreate();
        }
        else {
            SYN_VK_ASSERT_MSG(result, "Failed to present swapchain image");
        }
    }

    SwapChainSupportDetails SwapChain::QuerySwapChainSupport() {
        SwapChainSupportDetails details;
        vkGetPhysicalDeviceSurfaceCapabilitiesKHR(_physicalDevice.Handle(), _surface.Handle(), &details.capabilities);

        uint32_t formatCount;
        vkGetPhysicalDeviceSurfaceFormatsKHR(_physicalDevice.Handle(), _surface.Handle(), &formatCount, nullptr);

        if (formatCount != 0) {
            details.formats.resize(formatCount);
            vkGetPhysicalDeviceSurfaceFormatsKHR(_physicalDevice.Handle(), _surface.Handle(), &formatCount, details.formats.data());
        }

        uint32_t presentModeCount;
        vkGetPhysicalDeviceSurfacePresentModesKHR(_physicalDevice.Handle(), _surface.Handle(), &presentModeCount, nullptr);

        if (presentModeCount != 0) {
            details.presentModes.resize(presentModeCount);
            vkGetPhysicalDeviceSurfacePresentModesKHR(_physicalDevice.Handle(), _surface.Handle(), &presentModeCount, details.presentModes.data());
        }

        return details;
    }

    VkSurfaceFormatKHR SwapChain::ChooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats) {
        for (const auto& availableFormat : availableFormats) {
            if (availableFormat.format == VK_FORMAT_B8G8R8A8_UNORM && availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
                return availableFormat;
            }
        }
        return availableFormats[0];
    }

    VkPresentModeKHR SwapChain::ChooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes) {
        for (const auto& availablePresentMode : availablePresentModes) {
            if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR) {
                return availablePresentMode;
            }
        }
        return VK_PRESENT_MODE_FIFO_KHR;
    }

    VkExtent2D SwapChain::ChooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities) {
        if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max()) {
            return capabilities.currentExtent;
        }
        else {
            VkExtent2D actualExtent = _getWindowExtentCallback();
            actualExtent.width = std::clamp(actualExtent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
            actualExtent.height = std::clamp(actualExtent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);
            return actualExtent;
        }
    }
}