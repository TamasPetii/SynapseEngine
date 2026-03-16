#define VMA_IMPLEMENTATION
#include "Device.h"

namespace Syn::Vk {
    Device::Device(VkInstance instance, const PhysicalDevice& physicalDevice, std::span<const char*> requiredExtensions) {
        QueueFamilyIndices indices = physicalDevice.GetQueueFamilies();

        std::set<uint32_t> uniqueQueueFamilies = {
            indices.graphics.value(),
            indices.compute.value(),
            indices.transfer.value()
        };

        std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
        float queuePriority = 1.0f;
        for (uint32_t queueFamily : uniqueQueueFamilies)
        {
            VkDeviceQueueCreateInfo queueCreateInfo{ VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO };
            queueCreateInfo.queueFamilyIndex = queueFamily;
            queueCreateInfo.queueCount = 1;
            queueCreateInfo.pQueuePriorities = &queuePriority;
            queueCreateInfos.push_back(queueCreateInfo);
        }

        VkPhysicalDeviceMeshShaderFeaturesEXT meshFeatures{ VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MESH_SHADER_FEATURES_EXT };
        meshFeatures.meshShader = VK_TRUE;
        meshFeatures.taskShader = VK_TRUE;

        VkPhysicalDeviceShaderObjectFeaturesEXT shaderObjectFeatures{ VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_OBJECT_FEATURES_EXT };
        shaderObjectFeatures.shaderObject = VK_TRUE;

        VkPhysicalDeviceDescriptorBufferFeaturesEXT descriptorBufferFeatures{ VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DESCRIPTOR_BUFFER_FEATURES_EXT };
        descriptorBufferFeatures.descriptorBuffer = VK_TRUE;
        descriptorBufferFeatures.descriptorBufferPushDescriptors = VK_TRUE;

        VkPhysicalDeviceExtendedDynamicStateFeaturesEXT dynamicState1{ VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_EXTENDED_DYNAMIC_STATE_FEATURES_EXT };
        dynamicState1.extendedDynamicState = VK_TRUE;

        VkPhysicalDeviceExtendedDynamicState2FeaturesEXT dynamicState2{ VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_EXTENDED_DYNAMIC_STATE_2_FEATURES_EXT };
        dynamicState2.extendedDynamicState2 = VK_TRUE;
        dynamicState2.extendedDynamicState2LogicOp = VK_TRUE;
        dynamicState2.extendedDynamicState2PatchControlPoints = VK_TRUE;

        VkPhysicalDeviceExtendedDynamicState3FeaturesEXT dynamicState3{ VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_EXTENDED_DYNAMIC_STATE_3_FEATURES_EXT };
        dynamicState3.extendedDynamicState3PolygonMode = VK_TRUE;
        dynamicState3.extendedDynamicState3RasterizationSamples = VK_TRUE;
        dynamicState3.extendedDynamicState3ColorBlendEnable = VK_TRUE;
        dynamicState3.extendedDynamicState3LogicOpEnable = VK_TRUE;

        VkPhysicalDeviceSwapchainMaintenance1FeaturesEXT maintenance1Features{ VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SWAPCHAIN_MAINTENANCE_1_FEATURES_EXT };
        maintenance1Features.swapchainMaintenance1 = VK_TRUE;

        VkPhysicalDeviceVulkan13Features features13{ VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES };
        features13.synchronization2 = VK_TRUE;
        features13.dynamicRendering = VK_TRUE;
        features13.maintenance4 = VK_TRUE;

        VkPhysicalDeviceVulkan12Features features12{ VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES };
        features12.bufferDeviceAddress = VK_TRUE;
        features12.descriptorIndexing = VK_TRUE;
        features12.shaderSampledImageArrayNonUniformIndexing = VK_TRUE;
        features12.runtimeDescriptorArray = VK_TRUE;
        features12.descriptorBindingPartiallyBound = VK_TRUE;
        features12.descriptorBindingVariableDescriptorCount = VK_TRUE;
        features12.descriptorBindingSampledImageUpdateAfterBind = VK_TRUE;
        features12.timelineSemaphore = VK_TRUE;
        features12.scalarBlockLayout = VK_TRUE;
        features12.samplerFilterMinmax = VK_TRUE;
        features12.drawIndirectCount = VK_TRUE;
        features12.storageBuffer8BitAccess = VK_TRUE;
        features12.uniformAndStorageBuffer8BitAccess = VK_TRUE;

        VkPhysicalDeviceVulkan11Features features11{ VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_1_FEATURES };
        features11.shaderDrawParameters = VK_TRUE;

        VkPhysicalDeviceFeatures2 deviceFeatures{ VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2 };
        deviceFeatures.features.samplerAnisotropy = VK_TRUE;
        deviceFeatures.features.multiDrawIndirect = VK_TRUE;
        deviceFeatures.features.fillModeNonSolid = VK_TRUE;
        deviceFeatures.features.wideLines = VK_TRUE;
        deviceFeatures.features.geometryShader = VK_TRUE;
        deviceFeatures.features.pipelineStatisticsQuery = VK_TRUE;
        deviceFeatures.features.shaderInt64 = VK_TRUE;

        deviceFeatures.pNext = &features11;
        features11.pNext = &features12;
        features12.pNext = &features13;
        features13.pNext = &meshFeatures;
        meshFeatures.pNext = &shaderObjectFeatures;
        shaderObjectFeatures.pNext = &descriptorBufferFeatures;
        descriptorBufferFeatures.pNext = &dynamicState1;
        dynamicState1.pNext = &dynamicState2;
        dynamicState2.pNext = &dynamicState3;
        dynamicState3.pNext = &maintenance1Features;
        maintenance1Features.pNext = nullptr;

        VkDeviceCreateInfo createInfo{ VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO };
        createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
        createInfo.pQueueCreateInfos = queueCreateInfos.data();
        createInfo.enabledExtensionCount = static_cast<uint32_t>(requiredExtensions.size());
        createInfo.ppEnabledExtensionNames = requiredExtensions.data();

        createInfo.pNext = &deviceFeatures;

        SYN_VK_ASSERT_MSG(vkCreateDevice(physicalDevice.Handle(), &createInfo, nullptr, &_handle), "Failed to create Logical Device with Full Extra Features");

        volkLoadDevice(_handle);

        std::unordered_map<uint32_t, std::shared_ptr<ThreadSafeQueue>> createdQueues;

        auto getQueue = [&](uint32_t familyIndex) -> std::shared_ptr<ThreadSafeQueue> {
            if (createdQueues.contains(familyIndex)) {
                return createdQueues[familyIndex];
            }

            VkQueue queueHandle;
            vkGetDeviceQueue(_handle, familyIndex, 0, &queueHandle);

            auto newQueue = std::make_shared<ThreadSafeQueue>(queueHandle, familyIndex);
            createdQueues[familyIndex] = newQueue;
            return newQueue;
        };

        _graphicsQueue = getQueue(indices.graphics.value());

        if (indices.compute.has_value()) {
            _computeQueue = getQueue(indices.compute.value());
        }

        if (indices.transfer.has_value()) {
            _transferQueue = getQueue(indices.transfer.value());
        }

        InitVMA(instance, physicalDevice);
    }

    Device::~Device() {
        if (_allocator != VK_NULL_HANDLE)
            vmaDestroyAllocator(_allocator);

        if (_handle != VK_NULL_HANDLE)
            vkDestroyDevice(_handle, nullptr);
    }

    void Device::InitVMA(VkInstance instance, const PhysicalDevice& physicalDevice) {
        VmaVulkanFunctions vulkanFunctions{};
        vulkanFunctions.vkGetInstanceProcAddr = vkGetInstanceProcAddr;
        vulkanFunctions.vkGetDeviceProcAddr = vkGetDeviceProcAddr;
        vulkanFunctions.vkGetPhysicalDeviceProperties = vkGetPhysicalDeviceProperties;
        vulkanFunctions.vkGetPhysicalDeviceMemoryProperties = vkGetPhysicalDeviceMemoryProperties;
        vulkanFunctions.vkAllocateMemory = vkAllocateMemory;
        vulkanFunctions.vkFreeMemory = vkFreeMemory;
        vulkanFunctions.vkMapMemory = vkMapMemory;
        vulkanFunctions.vkUnmapMemory = vkUnmapMemory;
        vulkanFunctions.vkFlushMappedMemoryRanges = vkFlushMappedMemoryRanges;
        vulkanFunctions.vkInvalidateMappedMemoryRanges = vkInvalidateMappedMemoryRanges;
        vulkanFunctions.vkBindBufferMemory = vkBindBufferMemory;
        vulkanFunctions.vkBindImageMemory = vkBindImageMemory;
        vulkanFunctions.vkGetBufferMemoryRequirements = vkGetBufferMemoryRequirements;
        vulkanFunctions.vkGetImageMemoryRequirements = vkGetImageMemoryRequirements;
        vulkanFunctions.vkCreateBuffer = vkCreateBuffer;
        vulkanFunctions.vkDestroyBuffer = vkDestroyBuffer;
        vulkanFunctions.vkCreateImage = vkCreateImage;
        vulkanFunctions.vkDestroyImage = vkDestroyImage;
        vulkanFunctions.vkCmdCopyBuffer = vkCmdCopyBuffer;
        vulkanFunctions.vkGetBufferMemoryRequirements2KHR = vkGetBufferMemoryRequirements2;
        vulkanFunctions.vkGetImageMemoryRequirements2KHR = vkGetImageMemoryRequirements2;
        vulkanFunctions.vkBindBufferMemory2KHR = vkBindBufferMemory2;
        vulkanFunctions.vkBindImageMemory2KHR = vkBindImageMemory2;
        vulkanFunctions.vkGetPhysicalDeviceMemoryProperties2KHR = vkGetPhysicalDeviceMemoryProperties2;
        vulkanFunctions.vkGetDeviceBufferMemoryRequirements = vkGetDeviceBufferMemoryRequirements;
        vulkanFunctions.vkGetDeviceImageMemoryRequirements = vkGetDeviceImageMemoryRequirements;

        VmaAllocatorCreateInfo allocatorInfo{};
        allocatorInfo.physicalDevice = physicalDevice.Handle();
        allocatorInfo.device = _handle;
        allocatorInfo.instance = instance;
        allocatorInfo.flags = VMA_ALLOCATOR_CREATE_BUFFER_DEVICE_ADDRESS_BIT;
        allocatorInfo.pVulkanFunctions = &vulkanFunctions;
        allocatorInfo.vulkanApiVersion = VK_API_VERSION_1_4;

        SYN_VK_ASSERT_MSG(vmaCreateAllocator(&allocatorInfo, &_allocator), "Failed to create VMA Allocator");
    }
}