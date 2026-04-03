#include "PhysicalDevice.h"
#include "Engine/Logger/Logger.h"

namespace Syn::Vk {

    PhysicalDevice::PhysicalDevice(VkInstance instance, VkSurfaceKHR surface, std::span<const char*> requiredExtensions) {
        _handle = PickBestDevice(instance, surface, requiredExtensions);
        SYN_ASSERT(_handle != VK_NULL_HANDLE, "Failed to find a suitable GPU with Vulkan support");

        _descriptorBufferProperties = { VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DESCRIPTOR_BUFFER_PROPERTIES_EXT };

        VkPhysicalDeviceProperties2 props2{ VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2 };
        props2.pNext = &_descriptorBufferProperties;

        vkGetPhysicalDeviceProperties2(_handle, &props2);
        _properties = props2.properties;

        vkGetPhysicalDeviceMemoryProperties(_handle, &_memoryProperties);
        _indices = FindQueueFamilies(_handle, surface);

        uint32_t extCount;
        vkEnumerateDeviceExtensionProperties(_handle, nullptr, &extCount, nullptr);
        std::vector<VkExtensionProperties> availableExtensions(extCount);
        vkEnumerateDeviceExtensionProperties(_handle, nullptr, &extCount, availableExtensions.data());

        for (const auto& ext : availableExtensions) {
            _supportedExtensions.insert(ext.extensionName);
        }

        LogProperties();
    }

    bool PhysicalDevice::IsExtensionSupported(const char* extensionName) const {
        return _supportedExtensions.find(extensionName) != _supportedExtensions.end();
    }

    VkPhysicalDevice PhysicalDevice::PickBestDevice(VkInstance instance, VkSurfaceKHR surface, std::span<const char*> extensions) {
        uint32_t deviceCount = 0;
        vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);

        if (deviceCount == 0) return VK_NULL_HANDLE;

        std::vector<VkPhysicalDevice> devices(deviceCount);
        vkEnumeratePhysicalDevices(instance, &deviceCount, devices.data());

        for (const auto& device : devices) {
            if (CheckDeviceExtensionSupport(device, extensions)) {
                auto families = FindQueueFamilies(device, surface);
                if (families.IsComplete()) {
                    VkPhysicalDeviceProperties props;
                    vkGetPhysicalDeviceProperties(device, &props);
                    if (props.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) {
                        return device;
                    }
                }
            }
        }

        for (const auto& device : devices) {
            if (CheckDeviceExtensionSupport(device, extensions)) {
                auto families = FindQueueFamilies(device, surface);
                if (families.IsComplete()) return device;
            }
        }

        return VK_NULL_HANDLE;
    }

    QueueFamilyIndices PhysicalDevice::FindQueueFamilies(VkPhysicalDevice device, VkSurfaceKHR surface) {
        QueueFamilyIndices inds;

        uint32_t queueFamilyCount = 0;
        vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);
        std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
        vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

        for (uint32_t i = 0; i < queueFamilyCount; i++) {
            if (queueFamilies[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) {
                inds.graphics = i;
            }

            VkBool32 presentSupport = false;
            vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface, &presentSupport);
            if (presentSupport) {
                inds.present = i;
            }

            if (inds.graphics.has_value() && inds.present.has_value() &&
                inds.graphics.value() == inds.present.value()) {
                break;
            }
        }

        for (uint32_t i = 0; i < queueFamilyCount; i++) {
            if ((queueFamilies[i].queueFlags & VK_QUEUE_COMPUTE_BIT) &&
                !(queueFamilies[i].queueFlags & VK_QUEUE_GRAPHICS_BIT)) {
                inds.compute = i;
                break;
            }
        }

        if (!inds.compute.has_value()) {
            for (uint32_t i = 0; i < queueFamilyCount; i++) {
                if (queueFamilies[i].queueFlags & VK_QUEUE_COMPUTE_BIT) {
                    inds.compute = i;
                    break;
                }
            }
        }

        for (uint32_t i = 0; i < queueFamilyCount; i++) {
            if ((queueFamilies[i].queueFlags & VK_QUEUE_TRANSFER_BIT) &&
                !(queueFamilies[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) &&
                !(queueFamilies[i].queueFlags & VK_QUEUE_COMPUTE_BIT)) {
                inds.transfer = i;
                break;
            }
        }
        
        if (!inds.transfer.has_value()) {
            for (uint32_t i = 0; i < queueFamilyCount; i++) {
                if ((queueFamilies[i].queueFlags & VK_QUEUE_TRANSFER_BIT) &&
                    !(queueFamilies[i].queueFlags & VK_QUEUE_GRAPHICS_BIT)) {
                    inds.transfer = i;
                    break;
                }
            }
        }

        if (!inds.transfer.has_value()) {
            inds.transfer = inds.graphics;
        }

        return inds;
    }

    bool PhysicalDevice::CheckDeviceExtensionSupport(VkPhysicalDevice device, std::span<const char*> extensions) {
        uint32_t extCount;
        vkEnumerateDeviceExtensionProperties(device, nullptr, &extCount, nullptr);
        std::vector<VkExtensionProperties> availableExtensions(extCount);
        vkEnumerateDeviceExtensionProperties(device, nullptr, &extCount, availableExtensions.data());

        std::set<std::string> required(extensions.begin(), extensions.end());
        for (const auto& extension : availableExtensions) {
            required.erase(extension.extensionName);
        }
        return required.empty();
    }

    void PhysicalDevice::LogProperties() const {
        
        if constexpr (!Syn::EnableLogging) {
            return;
        }

        auto apiVer = _properties.apiVersion;
        std::string apiVersionStr = std::format("{}.{}.{}",
            VK_API_VERSION_MAJOR(apiVer),
            VK_API_VERSION_MINOR(apiVer),
            VK_API_VERSION_PATCH(apiVer));

        const char* deviceTypeStr = "Unknown";
        switch (_properties.deviceType) {
        case VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU: deviceTypeStr = "Discrete GPU"; break;
        case VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU: deviceTypeStr = "Integrated GPU"; break;
        case VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU: deviceTypeStr = "Virtual GPU"; break;
        case VK_PHYSICAL_DEVICE_TYPE_CPU: deviceTypeStr = "CPU"; break;
        }

        std::stringstream ss;
        ss << "\n-----------------------------------------\n";

        ss << std::format("GPU Device:      {}\n", _properties.deviceName);
        ss << std::format("GPU Type:        {}\n", deviceTypeStr);
        ss << std::format("API Ver:         {}\n", apiVersionStr);
        ss << std::format("Driver Ver:      {}\n", _properties.driverVersion);
        ss << std::format("Vendor ID:       {}\n", _properties.vendorID);

        ss << "\n"; // Kis elválasztás
        ss << std::format("Max Image Dim 2D:    {}\n", _properties.limits.maxImageDimension2D);
        ss << std::format("Max Push Constants:  {} bytes\n", _properties.limits.maxPushConstantsSize);
        ss << std::format("Max Sampler Aniso:   {}\n", _properties.limits.maxSamplerAnisotropy);
        ss << std::format("Timestamp Period:    {} ns\n", _properties.limits.timestampPeriod);

        ss << "\n--- Descriptor Buffer Properties ---\n";
        ss << std::format("Combined Image Sampler Size: {} bytes\n", _descriptorBufferProperties.combinedImageSamplerDescriptorSize);
        ss << std::format("Sampled Image Size:          {} bytes\n", _descriptorBufferProperties.sampledImageDescriptorSize);
        ss << std::format("Storage Image Size:          {} bytes\n", _descriptorBufferProperties.storageImageDescriptorSize);
        ss << std::format("Uniform Buffer Size:         {} bytes\n", _descriptorBufferProperties.uniformBufferDescriptorSize);
        ss << std::format("Storage Buffer Size:         {} bytes\n", _descriptorBufferProperties.storageBufferDescriptorSize);
        ss << std::format("Desc. Buffer Offset Align:   {} bytes\n", _descriptorBufferProperties.descriptorBufferOffsetAlignment);

        ss << "-----------------------------------------";

        // 3. Egyetlen log hívás a végén
        Logger::Get().Dispatch(LogLevel::Info, ss.str(), "Vulkan", 0);
    }
}