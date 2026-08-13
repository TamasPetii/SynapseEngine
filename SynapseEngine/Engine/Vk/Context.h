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
#include "VkCommon.h"

#include "Core/Instance.h"
#include "Core/PhysicalDevice.h"
#include "Core/Device.h"
#include "Presentation/Surface.h"
#include "Presentation/SwapChain.h"

namespace Syn::Vk {

    struct SYN_API ContextInitParams {
        bool enableValidation = true;

        std::function<std::vector<const char*>()> getSurfaceExtensionsCallback;
        std::function<void(VkInstance, VkSurfaceKHR*)> createSurfaceCallback;
        std::function<VkExtent2D()> getWindowExtentCallback;
    };

    class SYN_API Context {
    public:
        Context(const ContextInitParams& params);
        ~Context();

        void InitSwapChain(const ContextInitParams& params);

        auto GetInstance() const { return _instance.get(); }
        auto GetPhysicalDevice() const { return _physicalDevice.get(); }
        auto GetDevice() const { return _device.get(); }
        auto GetSurface() const { return _surface.get(); }
        auto GetSwapChain() const { return _swapChain.get(); }
    private:
        std::unique_ptr<Instance> _instance;
        std::unique_ptr<Surface> _surface;
        std::unique_ptr<PhysicalDevice> _physicalDevice;
        std::unique_ptr<Device> _device;
        std::unique_ptr<SwapChain> _swapChain;
    };
}