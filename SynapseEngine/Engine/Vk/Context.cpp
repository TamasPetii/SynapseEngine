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

#define VOLK_IMPLEMENTATION
#include "Context.h"

namespace Syn::Vk {

    static std::vector<const char*> GetRequiredExtensions() {
        return {
            VK_KHR_SWAPCHAIN_EXTENSION_NAME,
            VK_EXT_SWAPCHAIN_MAINTENANCE_1_EXTENSION_NAME,
            VK_KHR_PUSH_DESCRIPTOR_EXTENSION_NAME,
            VK_EXT_MESH_SHADER_EXTENSION_NAME,
            VK_EXT_SHADER_OBJECT_EXTENSION_NAME,
            VK_EXT_DESCRIPTOR_BUFFER_EXTENSION_NAME,
            VK_EXT_DESCRIPTOR_INDEXING_EXTENSION_NAME,
            VK_EXT_EXTENDED_DYNAMIC_STATE_EXTENSION_NAME,
            VK_EXT_EXTENDED_DYNAMIC_STATE_2_EXTENSION_NAME,
            VK_EXT_EXTENDED_DYNAMIC_STATE_3_EXTENSION_NAME,
            VK_EXT_MEMORY_BUDGET_EXTENSION_NAME,
            VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME,
            VK_KHR_SYNCHRONIZATION_2_EXTENSION_NAME,
            VK_KHR_BUFFER_DEVICE_ADDRESS_EXTENSION_NAME,
            VK_EXT_SAMPLER_FILTER_MINMAX_EXTENSION_NAME,
            VK_EXT_MUTABLE_DESCRIPTOR_TYPE_EXTENSION_NAME,
            VK_KHR_VULKAN_MEMORY_MODEL_EXTENSION_NAME,
            VK_EXT_SUBGROUP_SIZE_CONTROL_EXTENSION_NAME,
            VK_EXT_DESCRIPTOR_HEAP_EXTENSION_NAME,
            VK_EXT_DEVICE_GENERATED_COMMANDS_EXTENSION_NAME,
            VK_KHR_VIDEO_QUEUE_EXTENSION_NAME,
            VK_KHR_VIDEO_DECODE_QUEUE_EXTENSION_NAME,
            VK_KHR_VIDEO_DECODE_H264_EXTENSION_NAME
            /*, VK_AMDX_SHADER_ENQUEUE_EXTENSION_NAME */
        };
    }

    Context::Context(const ContextInitParams& params)
    {
        std::vector<const char*> instanceExts;
        if (params.getSurfaceExtensionsCallback) {
            instanceExts = params.getSurfaceExtensionsCallback();
        }
        _instance = std::make_unique<Instance>(params.enableValidation, instanceExts);

        SYN_ASSERT(params.createSurfaceCallback, "Surface creation callback is missing!");

        _surface = std::make_unique<Surface>(*_instance, params.createSurfaceCallback);

        auto extensions = GetRequiredExtensions();
        _physicalDevice = std::make_unique<PhysicalDevice>(_instance->Handle(), _surface->Handle(), extensions);
        _device = std::make_unique<Device>(_instance->Handle(), *_physicalDevice, extensions);
    }

    void Context::InitSwapChain(const ContextInitParams& params)
    {
        SYN_ASSERT(params.getWindowExtentCallback, "Window extent callback is missing!");
        _swapChain = std::make_unique<SwapChain>(*_physicalDevice, *_device, *_surface, params.getWindowExtentCallback);
    }

    Context::~Context() {
        _swapChain.reset();
        _device.reset();
        _physicalDevice.reset();
        _surface.reset();
        _instance.reset();
    }
}