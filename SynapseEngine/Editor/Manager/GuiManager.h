#pragma once
#include <vulkan/vulkan.h>
#include <vector>
#include <memory>
#include "GuiTextureManager.h"
#include "EditorCore/Api/IFileDialogApi.h"
#include "Editor/View/IGuiWindow.h"
#include "Editor/Workspace/IWorkspace.h"
#include <unordered_map>

struct GLFWwindow;

namespace Syn {
    class GuiManager {
    public:
        ~GuiManager();

        void Init(GLFWwindow* window, VkInstance instance, VkPhysicalDevice physicalDevice, VkDevice device, VkQueue graphicsQueue, uint32_t graphicsQueueFamily, uint32_t imageCount, VkFormat colorFormat);
        void Shutdown();

        void BeginFrame();
        void UpdateAndDraw();
        void EndFrame();
        void Render(VkCommandBuffer commandBuffer);

        void OnKey(int key, int scancode, int action, int mods);
        void OnMouseButton(int button, int action, int mods);
        void OnMouseMove(float x, float y);
        void OnScroll(float xOffset, float yOffset);
        void OnChar(unsigned int codepoint);

        bool WantsCaptureKeyboard() const;
        bool WantsCaptureMouse() const;


        GuiTextureManager* GetTextureManager() const { return _textureManager.get(); }
        IFileDialogApi* GetFileDialog() const { return _fileDialog.get(); }

        void SetWorkspace(EditorWorkspace workspace) { _currentWorkspace = workspace; }
        EditorWorkspace GetWorkspace() const { return _currentWorkspace; }

        template<typename TWindow, typename... Args>
        void AddGlobalWindow(Args&&... args) {
            _globalWindows.push_back(std::make_unique<TWindow>(std::forward<Args>(args)...));
        }

        void AddWorkspace(EditorWorkspace type, std::unique_ptr<IWorkspace> workspace) {
            workspace->Initialize();
            _workspaces[type] = std::move(workspace);
        }

    private:
        void SetStyle();
    private:
        VkFormat _colorFormat;
        GLFWwindow* _windowHandle = nullptr;
        VkDevice _device = VK_NULL_HANDLE;
        VkDescriptorPool _imguiPool = VK_NULL_HANDLE;
        std::unique_ptr<GuiTextureManager> _textureManager;
        std::unique_ptr<IFileDialogApi> _fileDialog;

        EditorWorkspace _currentWorkspace = EditorWorkspace::Scene;
        std::vector<std::unique_ptr<IGuiWindow>> _globalWindows;
        std::unordered_map<EditorWorkspace, std::unique_ptr<IWorkspace>> _workspaces;
    };
}