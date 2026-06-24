#include "GuiManager.h"
#include <vulkan/vulkan.h>
#include <imgui.h>

#include <imgui_impl_glfw.h>
#include <imgui_impl_vulkan.h>
#include "GuiTextureManager.h"
#include <print>
#include "Engine/ServiceLocator.h"
#include "Engine/FrameContext.h"
#include "Editor/FileDialog/ImGuiFileDialogImpl.h"
#include "EditorIcons.h"

namespace Syn {
    GuiManager::~GuiManager() {
        Shutdown();
    }

    void GuiManager::Init(GLFWwindow* window, VkInstance instance, VkPhysicalDevice physicalDevice, VkDevice device, VkQueue graphicsQueue, uint32_t graphicsQueueFamily, uint32_t imageCount, VkFormat colorFormat) {
        _device = device;
        _windowHandle = window;
        _colorFormat = colorFormat;
		_textureManager = std::make_unique<GuiTextureManager>();
        _fileDialog = std::make_unique<ImGuiFileDialogImpl>();

        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGuiIO& io = ImGui::GetIO();
        io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
        io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;

        ImGui_ImplGlfw_InitForVulkan(window, false);

        VkDescriptorPoolSize poolSizes[] =
        {
            { VK_DESCRIPTOR_TYPE_SAMPLER, 1000 },
            { VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000 },
            { VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1000 },
            { VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1000 },
            { VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 1000 },
            { VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 1000 },
            { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1000 },
            { VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1000 },
            { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1000 },
            { VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1000 },
            { VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1000 }
        };

        VkDescriptorPoolCreateInfo poolInfo = { .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO };
        poolInfo.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
        poolInfo.maxSets = 1000 * IM_ARRAYSIZE(poolSizes);
        poolInfo.poolSizeCount = (uint32_t)IM_ARRAYSIZE(poolSizes);
        poolInfo.pPoolSizes = poolSizes;
        vkCreateDescriptorPool(device, &poolInfo, nullptr, &_imguiPool);

        ImGui_ImplVulkan_InitInfo init_info = {};
        init_info.Instance = instance;
        init_info.PhysicalDevice = physicalDevice;
        init_info.Device = device;
        init_info.Queue = graphicsQueue;
        init_info.QueueFamily = graphicsQueueFamily;
        init_info.DescriptorPool = _imguiPool;
        init_info.MinImageCount = imageCount;
        init_info.ImageCount = imageCount;
        init_info.UseDynamicRendering = true;
        init_info.PipelineInfoMain.MSAASamples = VK_SAMPLE_COUNT_1_BIT;
        init_info.PipelineInfoMain.PipelineRenderingCreateInfo = { .sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO };
        init_info.PipelineInfoMain.PipelineRenderingCreateInfo.colorAttachmentCount = 1;
        init_info.PipelineInfoMain.PipelineRenderingCreateInfo.pColorAttachmentFormats = &_colorFormat;

        ImGui_ImplVulkan_LoadFunctions(VK_API_VERSION_1_4, [](const char* function_name, void* user_data) {
            return vkGetInstanceProcAddr(reinterpret_cast<VkInstance>(user_data), function_name);
            }, instance);

        ImGui_ImplVulkan_Init(&init_info);

        SetStyle();
    }

    void GuiManager::Shutdown() {
        vkDeviceWaitIdle(_device);

        _fileDialog.reset();
        _textureManager.reset();

        ImGui_ImplVulkan_Shutdown();

        if (_imguiPool != VK_NULL_HANDLE) {
            vkDestroyDescriptorPool(_device, _imguiPool, nullptr);
            _imguiPool = VK_NULL_HANDLE;
        }

        ImGui_ImplGlfw_Shutdown();
        ImGui::DestroyContext();

        _device = VK_NULL_HANDLE; 
    }

    void GuiManager::BeginFrame() {
        
        if (auto frameCtx = ServiceLocator::GetFrameContext())
            _textureManager->SetCurrentFrame(frameCtx->currentFrameIndex);

        ImGui_ImplVulkan_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
    }

    void GuiManager::UpdateAndDraw() {
        ImGuiViewport* viewport = ImGui::GetMainViewport();

        for (auto& window : _globalWindows) {
            window->UpdateAndDraw();
        }

        if (ImGui::BeginMainMenuBar()) {

            ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 0.0f);
            ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0.0f, 0.0f));
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 0, 0, 0));

            const char* wsScene = SYN_WS_SCENE;
            const char* wsModel = SYN_WS_MODEL;
            const char* wsMaterial = SYN_WS_MATERIAL;
            const char* wsTexture = SYN_WS_TEXTURE;

            ImVec2 btnPadding = ImGui::GetStyle().FramePadding;
            float totalWidth = ImGui::CalcTextSize(wsScene).x + ImGui::CalcTextSize(wsModel).x + ImGui::CalcTextSize(wsMaterial).x + ImGui::CalcTextSize(wsTexture).x + (btnPadding.x * 2.0f * 4.0f);

            ImGui::SetCursorPosX(ImGui::GetWindowWidth() - totalWidth - 10.0f);

            auto WorkspaceTab = [&](const char* label, EditorWorkspace ws) {
                bool isSelected = (_currentWorkspace == ws);

                if (isSelected) {
                    ImGui::PushStyleColor(ImGuiCol_Button, ImGui::GetStyleColorVec4(ImGuiCol_TabSelected));
                    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 1.0f, 1.0f));
                }
                else {
                    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 0, 0, 0));
                    ImGui::PushStyleColor(ImGuiCol_Text, ImGui::GetStyleColorVec4(ImGuiCol_TextDisabled));
                }

                if (ImGui::Button(label)) {
                    _currentWorkspace = ws;
                }

                ImGui::PopStyleColor(2);
                };

            WorkspaceTab(wsScene, EditorWorkspace::Scene);
            WorkspaceTab(wsModel, EditorWorkspace::Model);
            WorkspaceTab(wsMaterial, EditorWorkspace::Material);
            WorkspaceTab(wsTexture, EditorWorkspace::Texture);

            ImGui::PopStyleColor();
            ImGui::PopStyleVar(2);

            ImGui::EndMainMenuBar();
        }

        if (_fileDialog) {
            _fileDialog->Draw();
        }

        ImGui::SetNextWindowPos(viewport->WorkPos);
        ImGui::SetNextWindowSize(viewport->WorkSize);
        ImGui::SetNextWindowViewport(viewport->ID);

        ImGuiWindowFlags hostWindowFlags = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize |
                                           ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse |
                                           ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus |
                                           ImGuiWindowFlags_NoBackground;

        std::string hostWindowName = "";
        ImGuiID subDockspaceId = 0;

        switch (_currentWorkspace) {
            case EditorWorkspace::Scene:    hostWindowName = "HostWindow_Scene";    subDockspaceId = ImGui::GetID("DockSpace_Scene"); break;
            case EditorWorkspace::Texture:  hostWindowName = "HostWindow_Texture";  subDockspaceId = ImGui::GetID("DockSpace_Texture"); break;
            case EditorWorkspace::Material: hostWindowName = "HostWindow_Material"; subDockspaceId = ImGui::GetID("DockSpace_Material"); break;
            case EditorWorkspace::Model:    hostWindowName = "HostWindow_Model";    subDockspaceId = ImGui::GetID("DockSpace_Model"); break;
            default:                        hostWindowName = "HostWindow_Default";  subDockspaceId = ImGui::GetID("DockSpace_Default"); break;
        }

        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));

        if (ImGui::Begin(hostWindowName.c_str(), nullptr, hostWindowFlags)) {

            ImGui::DockSpace(subDockspaceId, ImVec2(0, 0), ImGuiDockNodeFlags_None);

            if (_workspaces.contains(_currentWorkspace)) {
                _workspaces[_currentWorkspace]->UpdateAndDraw();
            }
        }

        ImGui::End();
        ImGui::PopStyleVar();
    }

    void GuiManager::EndFrame() {
        ImGui::Render();

        ImGuiIO& io = ImGui::GetIO();
        if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
            ImGui::UpdatePlatformWindows();
            ImGui::RenderPlatformWindowsDefault();
        }
    }

    void GuiManager::Render(VkCommandBuffer commandBuffer) {
        ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), commandBuffer);
    }

    void GuiManager::OnKey(int key, int scancode, int action, int mods) {
        ImGui_ImplGlfw_KeyCallback(_windowHandle, key, scancode, action, mods);
    }

    void GuiManager::OnMouseButton(int button, int action, int mods) {
        ImGui_ImplGlfw_MouseButtonCallback(_windowHandle, button, action, mods);
    }

    void GuiManager::OnMouseMove(float x, float y) {
        ImGui_ImplGlfw_CursorPosCallback(_windowHandle, x, y);
    }

    void GuiManager::OnScroll(float xOffset, float yOffset) {
        ImGui_ImplGlfw_ScrollCallback(_windowHandle, xOffset, yOffset);
    }

    bool GuiManager::WantsCaptureKeyboard() const {
        return ImGui::GetCurrentContext() ? ImGui::GetIO().WantCaptureKeyboard : false;
    }

    bool GuiManager::WantsCaptureMouse() const {
        return ImGui::GetCurrentContext() ? ImGui::GetIO().WantCaptureMouse : false;
    }

    void GuiManager::SetStyle() { 
        ImGuiStyle& style = ImGui::GetStyle();
        ImVec4* colors = style.Colors;

        style.WindowPadding = ImVec2(8.0f, 8.0f);
        style.FramePadding = ImVec2(5.0f, 5.0f);
        style.CellPadding = ImVec2(4.0f, 2.0f);
        style.ItemSpacing = ImVec2(8.0f, 4.0f);
        style.ItemInnerSpacing = ImVec2(4.0f, 4.0f);
        style.TouchExtraPadding = ImVec2(0.0f, 0.0f);
        style.IndentSpacing = 12.0f;
        style.ScrollbarSize = 14.0f;
        style.GrabMinSize = 10.0f;
        style.WindowBorderSize = 1.0f;
        style.ChildBorderSize = 1.0f;
        style.PopupBorderSize = 1.0f;
        style.FrameBorderSize = 0.0f;
        style.TabBorderSize = 0.0f;
        style.WindowRounding = 2.0f;
        style.ChildRounding = 2.0f;
        style.FrameRounding = 3.0f;
        style.PopupRounding = 2.0f;
        style.ScrollbarRounding = 9.0f;
        style.GrabRounding = 2.0f;
        style.LogSliderDeadzone = 4.0f;
        style.TabRounding = 2.0f;

        colors[ImGuiCol_Text] = ImVec4(0.95f, 0.96f, 0.98f, 1.00f);
        colors[ImGuiCol_TextDisabled] = ImVec4(0.50f, 0.50f, 0.50f, 1.00f);
        colors[ImGuiCol_WindowBg] = ImVec4(0.11f, 0.11f, 0.11f, 1.00f);
        colors[ImGuiCol_ChildBg] = ImVec4(0.13f, 0.13f, 0.13f, 1.00f);
        colors[ImGuiCol_PopupBg] = ImVec4(0.08f, 0.08f, 0.08f, 0.94f);
        colors[ImGuiCol_Border] = ImVec4(0.06f, 0.06f, 0.06f, 0.80f);
        colors[ImGuiCol_BorderShadow] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
        colors[ImGuiCol_FrameBg] = ImVec4(0.05f, 0.05f, 0.05f, 0.54f);
        colors[ImGuiCol_FrameBgHovered] = ImVec4(0.19f, 0.19f, 0.19f, 0.54f);
        colors[ImGuiCol_FrameBgActive] = ImVec4(0.25f, 0.25f, 0.25f, 0.67f);
        colors[ImGuiCol_TitleBg] = ImVec4(0.08f, 0.08f, 0.08f, 1.00f);
        colors[ImGuiCol_TitleBgActive] = ImVec4(0.08f, 0.08f, 0.08f, 1.00f);
        colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.00f, 0.00f, 0.00f, 0.51f);
        colors[ImGuiCol_MenuBarBg] = ImVec4(0.10f, 0.10f, 0.10f, 1.00f);
        colors[ImGuiCol_ScrollbarBg] = ImVec4(0.02f, 0.02f, 0.02f, 0.53f);
        colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.31f, 0.31f, 0.31f, 1.00f);
        colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.41f, 0.41f, 0.41f, 1.00f);
        colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.51f, 0.51f, 0.51f, 1.00f);
        colors[ImGuiCol_CheckMark] = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
        colors[ImGuiCol_SliderGrab] = ImVec4(0.24f, 0.52f, 0.88f, 1.00f);
        colors[ImGuiCol_SliderGrabActive] = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
        colors[ImGuiCol_Button] = ImVec4(0.20f, 0.20f, 0.20f, 1.00f);
        colors[ImGuiCol_ButtonHovered] = ImVec4(0.28f, 0.28f, 0.28f, 1.00f);
        colors[ImGuiCol_ButtonActive] = ImVec4(0.06f, 0.53f, 0.98f, 1.00f);
        colors[ImGuiCol_Header] = ImVec4(0.20f, 0.20f, 0.20f, 1.00f);
        colors[ImGuiCol_HeaderHovered] = ImVec4(0.26f, 0.26f, 0.26f, 0.80f);
        colors[ImGuiCol_HeaderActive] = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
        colors[ImGuiCol_Separator] = ImVec4(0.08f, 0.08f, 0.08f, 1.00f);
        colors[ImGuiCol_SeparatorHovered] = ImVec4(0.10f, 0.10f, 0.10f, 0.78f);
        colors[ImGuiCol_SeparatorActive] = ImVec4(0.10f, 0.10f, 0.10f, 1.00f);
        colors[ImGuiCol_ResizeGrip] = ImVec4(0.26f, 0.59f, 0.98f, 0.25f);
        colors[ImGuiCol_ResizeGripHovered] = ImVec4(0.26f, 0.59f, 0.98f, 0.67f);
        colors[ImGuiCol_ResizeGripActive] = ImVec4(0.26f, 0.59f, 0.98f, 0.95f);
        colors[ImGuiCol_Tab] = ImVec4(0.11f, 0.11f, 0.11f, 0.86f);
        colors[ImGuiCol_TabHovered] = ImVec4(0.20f, 0.20f, 0.20f, 0.80f);
        colors[ImGuiCol_TabSelected] = ImVec4(0.18f, 0.18f, 0.18f, 1.00f);
        colors[ImGuiCol_TabSelectedOverline] = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
        colors[ImGuiCol_TabDimmed] = ImVec4(0.08f, 0.08f, 0.08f, 0.97f);
        colors[ImGuiCol_TabDimmedSelected] = ImVec4(0.12f, 0.12f, 0.12f, 1.00f);
        colors[ImGuiCol_TabDimmedSelectedOverline] = ImVec4(0.50f, 0.50f, 0.50f, 1.00f);
        colors[ImGuiCol_DockingPreview] = ImVec4(0.26f, 0.59f, 0.98f, 0.70f);
        colors[ImGuiCol_DockingEmptyBg] = ImVec4(0.20f, 0.20f, 0.20f, 1.00f);
        colors[ImGuiCol_PlotLines] = ImVec4(0.61f, 0.61f, 0.61f, 1.00f);
        colors[ImGuiCol_PlotLinesHovered] = ImVec4(1.00f, 0.43f, 0.35f, 1.00f);
        colors[ImGuiCol_PlotHistogram] = ImVec4(0.90f, 0.70f, 0.00f, 1.00f);
        colors[ImGuiCol_PlotHistogramHovered] = ImVec4(1.00f, 0.60f, 0.00f, 1.00f);
        colors[ImGuiCol_NavHighlight] = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
        colors[ImGuiCol_NavWindowingDimBg] = ImVec4(0.80f, 0.80f, 0.80f, 0.20f);
        colors[ImGuiCol_ModalWindowDimBg] = ImVec4(0.80f, 0.80f, 0.80f, 0.35f);
    }
}