#pragma once
#include "../EngineApi.h"
#include "../Logger/Checker.h"

#include "ValidationLayer.h"
#include "PhysicalDevice.h"
#include "ValidationLayer.h"

#include <vulkan/vulkan.h>
#include <functional>
#include <optional>
#include <set>



namespace Vk
{
	enum class ENGINE_API QueueType
	{
		GRAPHICS,
		PRESENTATION,
		COMPUTE,
		TRANSFER
	};

	class ENGINE_API Device
	{
	public:
		Device(const Vk::PhysicalDevice* const physicalDevice, std::span<const char*> deviceExtensions);
		~Device();
		VkDevice Value() const;
		VkQueue GetQueue(QueueType type) const;
		PFN_vkCmdPushDescriptorSetKHR vkCmdPushDescriptorSetKHR = nullptr;
	private:
		void Init(std::span<const char*> deviceExtensions);
		void Destroy();
	private:
	private:
		const Vk::PhysicalDevice* const physicalDevice;
		VkDevice device = VK_NULL_HANDLE;
		VkQueue graphicsQueue = VK_NULL_HANDLE;
		VkQueue presentQueue = VK_NULL_HANDLE;
		VkQueue computeQueue = VK_NULL_HANDLE;
		VkQueue transferQueue = VK_NULL_HANDLE;
	};
}


