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
#include <string>
#include <functional>
#include <utility>
#include <memory>

typedef struct VkInstance_T* VkInstance;
typedef struct VkSurfaceKHR_T* VkSurfaceKHR;

namespace Syn 
{
    struct WindowConfig {
        std::string title = "SynapseWindow";
        uint32_t width = 1280;
        uint32_t height = 720;
        bool resizable = true;
    };

    struct WindowCallbacks {
        std::function<void(uint32_t width, uint32_t height)> OnResize;
        std::function<void()> OnClose;
        std::function<void(int key, int scancode, int action, int mods)> OnKey;
        std::function<void(float x, float y)> OnMouseMove;
        std::function<void(int button, int action, int mods)> OnMouseButton;
        std::function<void(float xOffset, float yOffset)> OnScroll;
        std::function<void(unsigned int codepoint)> OnChar;
    };

    struct WindowData
    {
        WindowConfig config;
        WindowCallbacks callbacks;
    };

    class Window {
    public:
        virtual ~Window() = default;

        virtual void Update() = 0;
        virtual bool ShouldClose() const = 0;

        virtual uint32_t GetWidth() const = 0;
        virtual uint32_t GetHeight() const = 0;
        virtual std::pair<uint32_t, uint32_t> GetSize() const = 0;

        virtual std::vector<const char*> GetRequiredExtensions() const = 0;

        virtual void CreateSurface(VkInstance instance, VkSurfaceKHR* surface) = 0;
        virtual void SetCallbacks(const WindowCallbacks& callbacks) = 0;
        virtual void SetIcon(uint32_t width, uint32_t height, const uint8_t* pixels) = 0;
        virtual void* GetNativePointer() const = 0;

        static std::unique_ptr<Window> Create(const WindowConfig& config);
    protected:
        WindowData _data;
    };
}