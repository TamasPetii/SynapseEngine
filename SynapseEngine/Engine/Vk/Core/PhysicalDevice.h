#pragma once
#include "../VkCommon.h"
#include "QueueFamily.h"

namespace Syn::Vk {
    class SYN_API PhysicalDevice {
    public:
        PhysicalDevice(VkInstance instance, VkSurfaceKHR surface, std::span<const char*> requiredExtensions);

        VkPhysicalDevice Handle() const { return _handle; }
        const QueueFamilyIndices& GetQueueFamilies() const { return _indices; }
        const VkPhysicalDeviceProperties& GetProperties() const { return _properties; }
        const VkPhysicalDeviceDescriptorBufferPropertiesEXT& GetDescriptorBufferProperties() const { return _descriptorBufferProperties; }
        bool IsExtensionSupported(const char* extensionName) const;
    private:
        void LogProperties() const;
        VkPhysicalDevice PickBestDevice(VkInstance instance, VkSurfaceKHR surface, std::span<const char*> extensions);
        QueueFamilyIndices FindQueueFamilies(VkPhysicalDevice device, VkSurfaceKHR surface);
        bool CheckDeviceExtensionSupport(VkPhysicalDevice device, std::span<const char*> extensions);
    private:
        VkPhysicalDevice _handle = VK_NULL_HANDLE;
        QueueFamilyIndices _indices;
        VkPhysicalDeviceProperties _properties{};
        VkPhysicalDeviceMemoryProperties _memoryProperties{};
        VkPhysicalDeviceDescriptorBufferPropertiesEXT _descriptorBufferProperties{};
        std::set<std::string> _supportedExtensions;
    };
}