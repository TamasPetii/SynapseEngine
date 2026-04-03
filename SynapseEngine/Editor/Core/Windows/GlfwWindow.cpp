#include "GlfwWindow.h"
#include <print>

#define GLFW_INCLUDE_VULKAN
#define GLFW_EXPOSE_NATIVE_WIN32
#include <glfw/glfw3.h>
#include <glfw/glfw3native.h>
#include <dwmapi.h>
#pragma comment(lib, "Dwmapi.lib")

#include <volk.h>

namespace Syn {

    static void GLFWErrorCallback(int error, const char* description) {
        std::println(stderr, "GLFW Error ({}): {}", error, description);
    }

    GlfwWindow::GlfwWindow(const WindowConfig& config) {
        Init(config);
    }

    GlfwWindow::~GlfwWindow() {
        Shutdown();
    }

    void GlfwWindow::Init(const WindowConfig& config)
    {
        _data.config = config;

        glfwSetErrorCallback(GLFWErrorCallback);

        SYN_ASSERT(glfwInit(), "Failed to initialize GLFW!");

        GLFWmonitor* primaryMonitor = glfwGetPrimaryMonitor();
        const GLFWvidmode* videoMode = glfwGetVideoMode(primaryMonitor);
        const int32_t width = static_cast<int32_t>(videoMode->width * 0.9f);
        const int32_t height = static_cast<int32_t>(videoMode->height * 0.9f);

        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        glfwWindowHint(GLFW_RESIZABLE, config.resizable ? GLFW_TRUE : GLFW_FALSE);

        _window = glfwCreateWindow(
            width,
            height,
            config.title.c_str(),
            nullptr,
            nullptr
        );

        SYN_ASSERT(_window, "Failed to create GLFW window!");

        glfwSetWindowUserPointer(_window, &_data);

        glfwSetWindowSizeCallback(_window, [](GLFWwindow* window, int width, int height) {
            WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);
            data.config.width = width;
            data.config.height = height;

            if (data.callbacks.OnResize) {
                data.callbacks.OnResize(width, height);
            }
            });

        glfwSetWindowCloseCallback(_window, [](GLFWwindow* window) {
            WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);
            if (data.callbacks.OnClose) {
                data.callbacks.OnClose();
            }
            });

        glfwSetKeyCallback(_window, [](GLFWwindow* window, int key, int scancode, int action, int mods) {
            WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);
            if (data.callbacks.OnKey) {
                data.callbacks.OnKey(key, scancode, action, mods);
            }
            });

        glfwSetCursorPosCallback(_window, [](GLFWwindow* window, double xPos, double yPos) {
            WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);
            if (data.callbacks.OnMouseMove) {
                data.callbacks.OnMouseMove((float)xPos, (float)yPos);
            }
            });

        glfwSetMouseButtonCallback(_window, [](GLFWwindow* window, int button, int action, int mods) {
            WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);
            if (data.callbacks.OnMouseButton) {
                data.callbacks.OnMouseButton(button, action, mods);
            }
            });

        glfwSetScrollCallback(_window, [](GLFWwindow* window, double xOffset, double yOffset) {
            WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);
            if (data.callbacks.OnScroll) {
                data.callbacks.OnScroll((float)xOffset, (float)yOffset);
            }
            });

        glfwSetFramebufferSizeCallback(_window, [](GLFWwindow* window, int width, int height) {
            WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);

            if (data.callbacks.OnResize) {
                data.callbacks.OnResize(width, height);
            }
            });

        HWND hwnd = glfwGetWin32Window(_window);
        COLORREF color = RGB(0, 0, 0);
        const DWORD DWMWA_CAPTION_COLOR = 35;
        HRESULT hr = DwmSetWindowAttribute(hwnd, DWMWA_CAPTION_COLOR, &color, sizeof(color));

        const int32_t windowLeft = videoMode->width / 2 - width / 2;
        const int32_t windowTop = videoMode->height / 2 - height / 2;
        glfwSetWindowPos(_window, windowLeft, windowTop);
    }

    void GlfwWindow::Shutdown() {
        if (_window) {
            glfwDestroyWindow(_window);
            _window = nullptr;
        }
        glfwTerminate();
    }

    void GlfwWindow::Update() {
        glfwPollEvents();
    }

    bool GlfwWindow::ShouldClose() const {
        return glfwWindowShouldClose(_window);
    }

    std::vector<const char*> GlfwWindow::GetRequiredExtensions() const {
        uint32_t glfwExtensionCount = 0;
        const char** glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);
        std::vector<const char*> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);
        return extensions;
    }

    void GlfwWindow::CreateSurface(VkInstance instance, VkSurfaceKHR* surface) {
        SYN_VK_ASSERT_MSG(
            glfwCreateWindowSurface(instance, _window, nullptr, surface),
            "Failed to create Vulkan Window Surface!"
        );
    }

    void GlfwWindow::SetCallbacks(const WindowCallbacks& callbacks) {
        _data.callbacks = callbacks;
    }
}