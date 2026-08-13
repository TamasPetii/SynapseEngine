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
#include "Window.h"
#include "Engine/SynMacro.h"

struct GLFWwindow;

namespace Syn {

    class GlfwWindow : public Window {
    public:
        GlfwWindow(const WindowConfig& config);
        ~GlfwWindow() override;

        void Update() override;
        bool ShouldClose() const override;

        uint32_t GetWidth() const override { return _data.config.width; }
        uint32_t GetHeight() const override { return _data.config.height; }
        std::pair<uint32_t, uint32_t> GetSize() const override { return { GetWidth(), GetHeight()}; }   

        std::vector<const char*> GetRequiredExtensions() const override;

        void CreateSurface(VkInstance instance, VkSurfaceKHR* surface) override;
        void SetCallbacks(const WindowCallbacks& callbacks) override;
        void SetIcon(uint32_t width, uint32_t height, const uint8_t* pixels) override;
        void* GetNativePointer() const override { return _window; }
    private:
        void Init(const WindowConfig& config);
        void Shutdown();
    private:
        GLFWwindow* _window = nullptr;
    };
}