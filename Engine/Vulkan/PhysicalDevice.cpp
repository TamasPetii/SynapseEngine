#include "PhysicalDevice.h"

Vk::PhysicalDevice::PhysicalDevice(const Vk::Instance* const instance, const Vk::Surface* const surface, std::span<const char*> deviceExtensions) :
	instance(instance),
	surface(surface)
{
	Init(deviceExtensions);
}

Vk::PhysicalDevice::~PhysicalDevice()
{
	Destroy();
}

VkPhysicalDevice Vk::PhysicalDevice::Value() const
{
	return physicalDevice;
}

const Vk::QueueFamilyIndices& Vk::PhysicalDevice::GetQueueFamilyIndices() const
{
	return queueFamilyIndices;
}

void Vk::PhysicalDevice::Init(std::span<const char*> deviceExtensions)
{
	uint32_t physicalDeviceCount = 0;
	vkEnumeratePhysicalDevices(instance->Value(), &physicalDeviceCount, nullptr);

	CHECK_ERROR_NULL_MESSAGE(physicalDeviceCount, "Failed to find GPUs with vulkan support!");
		
	std::vector<VkPhysicalDevice> physicalDevices(physicalDeviceCount);
	vkEnumeratePhysicalDevices(instance->Value(), &physicalDeviceCount, physicalDevices.data());

	for (auto& physicalDevice : physicalDevices)
	{
		if (IsDeviceSuitable(physicalDevice, deviceExtensions))
		{
			this->physicalDevice = physicalDevice;
			this->queueFamilyIndices = FindQueueFamilies(physicalDevice);
			break;
		}
	}

	CHECK_ERROR_NULL_MESSAGE(this->physicalDevice, "Failed to find a suitable GPU!");

    //-------------------------------------------------------------------

    // Get physical device properties
    VkPhysicalDeviceProperties deviceProperties;
    vkGetPhysicalDeviceProperties(physicalDevice, &deviceProperties);

    // Get physical device limits
    VkPhysicalDeviceLimits limits = deviceProperties.limits;

    // Get extension properties to check for VK_EXT_descriptor_indexing
    uint32_t extensionCount;
    vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &extensionCount, nullptr);
    std::vector<VkExtensionProperties> extensions(extensionCount);
    vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &extensionCount, extensions.data());

	std::cout << std::format("Max Uniform Buffer Bindings (per pipeline): {}\n", limits.maxDescriptorSetUniformBuffers);
	std::cout << std::format("Max Dynamic Uniform Buffer Bindings (per pipeline): {}\n", limits.maxDescriptorSetUniformBuffersDynamic);
	std::cout << std::format("Max Uniform Buffer Range (bytes): {}\n", limits.maxUniformBufferRange);

	std::cout << std::format("Max Storage Buffer Bindings (per pipeline): {}\n", limits.maxDescriptorSetStorageBuffers);
	std::cout << std::format("Max Dynamic Storage Buffer Bindings (per pipeline): {}\n", limits.maxDescriptorSetStorageBuffersDynamic);
	std::cout << std::format("Max Storage Buffer Range (bytes): {}\n", limits.maxStorageBufferRange);

	std::cout << std::format("Max Sampled Image Bindings (per pipeline): {}\n", limits.maxDescriptorSetSampledImages);
	std::cout << std::format("Max Storage Image Bindings (per pipeline): {}\n", limits.maxDescriptorSetStorageImages);
	std::cout << std::format("Max Sampler Bindings (per pipeline): {}\n", limits.maxDescriptorSetSamplers);
	std::cout << std::format("Max Input Attachment Bindings (per pipeline): {}\n", limits.maxDescriptorSetInputAttachments);

	std::cout << std::format("Max Descriptor Sets (per pool): {}\n", limits.maxBoundDescriptorSets);
	std::cout << std::format("Max Descriptor Set Allocations: {}\n",
		limits.maxDescriptorSetUniformBuffers +
		limits.maxDescriptorSetStorageBuffers +
		limits.maxDescriptorSetSampledImages +
		limits.maxDescriptorSetStorageImages +
		limits.maxDescriptorSetSamplers +
		limits.maxDescriptorSetInputAttachments);

	std::cout << std::format("Max Push Constants Size (bytes): {}\n", limits.maxPushConstantsSize);

	std::cout << std::format("Max Image Dimension 1D: {}\n", limits.maxImageDimension1D);
	std::cout << std::format("Max Image Dimension 2D: {}\n", limits.maxImageDimension2D);
	std::cout << std::format("Max Image Dimension 3D: {}\n", limits.maxImageDimension3D);
	std::cout << std::format("Max Image Dimension Cube: {}\n", limits.maxImageDimensionCube);
	std::cout << std::format("Max Image Array Layers: {}\n", limits.maxImageArrayLayers);
	std::cout << std::format("Max Texel Buffer Elements: {}\n", limits.maxTexelBufferElements);
	std::cout << std::format("Max Sampler Allocation Count: {}\n", limits.maxSamplerAllocationCount);

	std::cout << std::format("Max Compute Wrok Group Count: {}, {}, {}\n", limits.maxComputeWorkGroupCount[0], limits.maxComputeWorkGroupCount[1], limits.maxComputeWorkGroupCount[2]);
	std::cout << std::format("Max Compute Wrok Group Size: {}, {}, {}\n", limits.maxComputeWorkGroupSize[0], limits.maxComputeWorkGroupSize[1], limits.maxComputeWorkGroupSize[2]);
	std::cout << std::format("Max Compute Work Group Invocation {}\n", limits.maxComputeWorkGroupInvocations);
}

void Vk::PhysicalDevice::Destroy()
{
}

bool Vk::PhysicalDevice::IsDeviceSuitable(VkPhysicalDevice physicalDevice, std::span<const char*> deviceExtensions)
{
	return FindQueueFamilies(physicalDevice).IsComplete() &&
		CheckDeviceExtensionSupport(physicalDevice, deviceExtensions) &&
		CheckSwapChainSupport(physicalDevice) &&
		CheckAnisotropySupport(physicalDevice);
}

Vk::QueueFamilyIndices Vk::PhysicalDevice::FindQueueFamilies(VkPhysicalDevice physicalDevice)
{
	QueueFamilyIndices indices;

	uint32_t queueFamilyCount = 0;
	vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, nullptr);

	std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
	vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, queueFamilies.data());

	for (uint32_t i = 0; i < queueFamilyCount; ++i) {
		std::cout << "Queue Family Index: " << i << " - Flags: ";
		VkQueueFlags flags = queueFamilies[i].queueFlags;

		if (flags & VK_QUEUE_GRAPHICS_BIT) {
			std::cout << "GRAPHICS ";
		}
		if (flags & VK_QUEUE_COMPUTE_BIT) {
			std::cout << "COMPUTE ";
		}
		if (flags & VK_QUEUE_TRANSFER_BIT) {
			std::cout << "TRANSFER ";
		}
		if (flags & VK_QUEUE_SPARSE_BINDING_BIT) {
			std::cout << "SPARSE_BINDING ";
		}
		if (flags & VK_QUEUE_PROTECTED_BIT) {
			std::cout << "PROTECTED ";
		}
		if (flags == 0) {
			std::cout << "NONE";
		}
		std::cout << std::endl;
	}

	for (int i = 0; i < queueFamilies.size(); i++)
	{
		if (queueFamilies[i].queueFlags & VK_QUEUE_GRAPHICS_BIT)
		{
			indices.graphicsFamily = i;
		}

		if (queueFamilies[i].queueFlags & VK_QUEUE_COMPUTE_BIT)
		{
			indices.computeFamily = i;
		}

		if (queueFamilies[i].queueFlags & VK_QUEUE_TRANSFER_BIT &&
			indices.graphicsFamily.has_value() && i != indices.graphicsFamily)
		{
			indices.transferFamily = i;
		}

		VkBool32 presentSupport = false;
		vkGetPhysicalDeviceSurfaceSupportKHR(physicalDevice, i, surface->Value(), &presentSupport);

		if (!indices.presentFamily.has_value() && presentSupport)
		{
			indices.presentFamily = i;
		}

		if (indices.IsComplete())
		{
			break;
		}
	}

	return indices;
}

bool Vk::PhysicalDevice::CheckDeviceExtensionSupport(VkPhysicalDevice physicalDevice, std::span<const char*> deviceExtensions)
{
	uint32_t extensionCount;
	vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &extensionCount, nullptr);

	std::vector<VkExtensionProperties> availableExtensions(extensionCount);
	vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &extensionCount, availableExtensions.data());

	std::set<std::string> requiredExtensions(deviceExtensions.begin(), deviceExtensions.end());

	for (const auto& extension : availableExtensions) {
		requiredExtensions.erase(extension.extensionName);
	}

	return requiredExtensions.empty();
}

bool Vk::PhysicalDevice::CheckSwapChainSupport(VkPhysicalDevice physicalDevice)
{
	//Surface Formats
	uint32_t formatCount;
	vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface->Value(), &formatCount, nullptr);

	//Surface Present Modes
	uint32_t presentModeCount;
	vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, surface->Value(), &presentModeCount, nullptr);

	return formatCount > 0 && presentModeCount > 0;
}

bool Vk::PhysicalDevice::CheckAnisotropySupport(VkPhysicalDevice physicalDevice)
{
	VkPhysicalDeviceFeatures supportedFeatures;
	vkGetPhysicalDeviceFeatures(physicalDevice, &supportedFeatures);

	return supportedFeatures.samplerAnisotropy;
}
