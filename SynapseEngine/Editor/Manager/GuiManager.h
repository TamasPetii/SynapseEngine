// Editor/UI/GuiManager.h
#pragma once
#include <vulkan/vulkan.h>
#include <vector>
#include <memory>
#include "Editor/View/IGuiWindow.h"

struct GLFWwindow;

namespace Syn {
    class GuiManager {
    public:
        void Init(GLFWwindow* window, VkInstance instance, VkPhysicalDevice physicalDevice, VkDevice device, VkQueue graphicsQueue, uint32_t imageCount, VkFormat colorFormat);
        void Shutdown();

        void BeginFrame();
        void UpdateAndDraw();
        void EndFrame();
        void Render(VkCommandBuffer commandBuffer);

        void OnKey(int key, int scancode, int action, int mods);
        void OnMouseButton(int button, int action, int mods);
        void OnMouseMove(float x, float y);
        void OnScroll(float xOffset, float yOffset);

        bool WantsCaptureKeyboard() const;
        bool WantsCaptureMouse() const;

        template<typename TWindow, typename... Args>
        void AddWindow(Args&&... args) {
            _windows.push_back(std::make_unique<TWindow>(std::forward<Args>(args)...));
        }
    private:
        void SetStyle();
    private:
        VkFormat _colorFormat;
        GLFWwindow* _windowHandle = nullptr;
        VkDevice _device = VK_NULL_HANDLE;
        VkDescriptorPool _imguiPool = VK_NULL_HANDLE;
        std::vector<std::unique_ptr<IGuiWindow>> _windows;
    };
}