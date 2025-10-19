#include "Device.h"

Vk::Device::Device(const Vk::PhysicalDevice* const physicalDevice, std::span<const char*> deviceExtensions) :
	physicalDevice(physicalDevice)
{
	Init(deviceExtensions);
}

Vk::Device::~Device()
{
	Destroy();
}

VkDevice Vk::Device::Value() const
{
	return device;
}

VkQueue Vk::Device::GetQueue(QueueType type) const
{
	switch (type)
	{
	case QueueType::GRAPHICS: return graphicsQueue; break;
	case QueueType::PRESENTATION: return presentQueue; break;
	case QueueType::COMPUTE: return computeQueue; break;
	case QueueType::TRANSFER: return transferQueue; break;
	default:
		CHECK_ERROR_NULL_MESSAGE(0, "QueueType not supported!");
	}
}

void Vk::Device::Init(std::span<const char*> deviceExtensions)
{
	QueueFamilyIndices indices = physicalDevice->GetQueueFamilyIndices();

	std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
	std::set<uint32_t> uniqueQueueFamilies = { indices.graphicsFamily.value(), indices.computeFamily.value(), indices.transferFamily.value(), indices.presentFamily.value() };

	for (uint32_t queueFamily : uniqueQueueFamilies)
	{
		VkDeviceQueueCreateInfo queueCreateInfo = {};
		queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		queueCreateInfo.queueFamilyIndex = queueFamily;
		queueCreateInfo.queueCount = 1;
		float queuePriority = 1.0f;
		queueCreateInfo.pQueuePriorities = &queuePriority;
		queueCreateInfos.push_back(queueCreateInfo);
	}

	VkPhysicalDeviceFeatures deviceFeatures{};
	deviceFeatures.samplerAnisotropy = VK_TRUE;
	deviceFeatures.multiDrawIndirect = VK_TRUE;
	deviceFeatures.fillModeNonSolid = VK_TRUE;
	deviceFeatures.wideLines = VK_TRUE;
	deviceFeatures.geometryShader = VK_TRUE;

	VkPhysicalDeviceVulkan11Features deviceFeatures11{};
	deviceFeatures11.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_1_FEATURES;
	deviceFeatures11.shaderDrawParameters = VK_TRUE;

	VkPhysicalDeviceVulkan12Features deviceFeatures12{};
	deviceFeatures12.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES;
	deviceFeatures12.bufferDeviceAddress = VK_TRUE;
	deviceFeatures12.descriptorIndexing = VK_TRUE;
	deviceFeatures12.descriptorBindingSampledImageUpdateAfterBind = VK_TRUE;
	deviceFeatures12.descriptorBindingPartiallyBound = VK_TRUE;
	deviceFeatures12.descriptorBindingVariableDescriptorCount = VK_TRUE;
	deviceFeatures12.runtimeDescriptorArray = VK_TRUE;
	deviceFeatures12.drawIndirectCount = VK_TRUE;
	deviceFeatures12.scalarBlockLayout = VK_TRUE;
	deviceFeatures12.samplerFilterMinmax = VK_TRUE;
	deviceFeatures12.pNext = &deviceFeatures11;

	VkPhysicalDeviceVulkan13Features deviceFeatures13 = {};
	deviceFeatures13.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES;
	deviceFeatures13.dynamicRendering = VK_TRUE;
	deviceFeatures13.synchronization2 = VK_TRUE;
	deviceFeatures13.pNext = &deviceFeatures12;

	// Add VK_EXT_shader_atomic_float2 features
	VkPhysicalDeviceShaderAtomicFloat2FeaturesEXT atomicFloat2Features{};
	atomicFloat2Features.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_ATOMIC_FLOAT_2_FEATURES_EXT;
	atomicFloat2Features.shaderBufferFloat32AtomicMinMax = VK_TRUE; // Enable 32-bit float atomic min/max
	atomicFloat2Features.pNext = &deviceFeatures13; // Chain to Vulkan 1.3 features

	VkPhysicalDeviceFeatures2 deviceFeatures2 = {};
	deviceFeatures2.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;
	deviceFeatures2.pNext = &atomicFloat2Features;
	deviceFeatures2.features = deviceFeatures;

	VkDeviceCreateInfo createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
	createInfo.pQueueCreateInfos = queueCreateInfos.data();
	createInfo.enabledExtensionCount = static_cast<uint32_t>(deviceExtensions.size());
	createInfo.ppEnabledExtensionNames = deviceExtensions.data();
	createInfo.pNext = &deviceFeatures2;

	if (ValidationLayer::ValidationLayerEnabled())
	{
		auto validationLayers = ValidationLayer::GetValidationLayers();
		createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
		createInfo.ppEnabledLayerNames = validationLayers.data();
	}
	else
	{
		createInfo.enabledLayerCount = 0;
	}

	VK_CHECK_MESSAGE(vkCreateDevice(physicalDevice->Value(), &createInfo, nullptr, &device), "Failed to create logical device!");

	vkCmdPushDescriptorSetKHR = reinterpret_cast<PFN_vkCmdPushDescriptorSetKHR>(vkGetDeviceProcAddr(device, "vkCmdPushDescriptorSetKHR"));

	if (!vkCmdPushDescriptorSetKHR)
	{
		throw std::runtime_error("Failed to load vkCmdPushDescriptorSetKHR — VK_KHR_push_descriptor not enabled or unsupported!");
	}

	vkGetDeviceQueue(device, indices.graphicsFamily.value(), 0, &graphicsQueue);
	vkGetDeviceQueue(device, indices.presentFamily.value(), 0, &presentQueue);
	vkGetDeviceQueue(device, indices.computeFamily.value(), 0, &computeQueue);
	vkGetDeviceQueue(device, indices.transferFamily.value(), 0, &transferQueue);
}

void Vk::Device::Destroy()
{
	if(device != VK_NULL_HANDLE)
		vkDestroyDevice(device, nullptr);

	device = VK_NULL_HANDLE;
}
