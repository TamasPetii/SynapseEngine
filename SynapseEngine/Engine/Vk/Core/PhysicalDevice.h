// Copyright (C) 2026 Tamás Péter
// This file is part of SynapseEngine.
//
// SynapseEngine is free software: you can redistribute it and/or modify
// it under the terms of the GNU Affero General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// SynapseEngine is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License
// along with SynapseEngine. If not, see <https://www.gnu.org/licenses/>.

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
        VkPhysicalDeviceMaintenance3Properties _maintenance3Properties{};
        std::set<std::string> _supportedExtensions;
    };
}