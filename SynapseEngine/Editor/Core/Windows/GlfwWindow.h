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
    private:
        void Init(const WindowConfig& config);
        void Shutdown();
    private:
        GLFWwindow* _window = nullptr;
    };
}