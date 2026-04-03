#include "Synapse.h"
#include "Engine/SynMacro.h"
#include "Engine/Vk/Context.h"
#include <GLFW/glfw3.h>

#include "Editor/View/Transform/TransformView.h"
#include "EditorCore/ViewModels/Transform/TransformViewModel.h"

#include "Editor/View/Viewport/ViewportView.h"
#include "EditorCore/ViewModels/Viewport/ViewportViewModel.h"

#include "Editor/View/Settings/SettingsView.h"
#include "EditorCore/ViewModels/Settings/SettingsViewModel.h"

#include "Manager/GuiTextureManager.h"

Synapse::Synapse(const Syn::ApplicationConfig& config)
    : Syn::Application(config)
{
}

Synapse::~Synapse() {
}

void Synapse::OnInit() {
    Syn::EngineInitParams params;

    params.createSurfaceCallback = [&](VkInstance instance, VkSurfaceKHR* surface) {
        GetWindow().CreateSurface(instance, surface);
    };

    params.getSurfaceExtensionsCallback = [&]() {
        return GetWindow().GetRequiredExtensions();
    };

    params.getWindowExtentCallback = [&]() {
        return GetWindow().GetSize();
        };

#ifndef SYN_PERFORMANCE

    params.onRenderGuiCallback = [&](VkCommandBuffer cmd) {
        if (_guiManager) {
            _guiManager->Render(cmd);
        }
        };

    params.onGuiFlushCallback = [&](uint32_t frameIndex) {
        Syn::GuiTextureManager::Get().FlushQueue(frameIndex);
        };
#endif

    _engine = std::make_unique<Syn::Engine>(params);

#ifndef SYN_PERFORMANCE
    _editorApi = std::make_unique<Syn::EditorApiImpl>(_engine.get());

    auto vkContext = _engine->GetVkContext();
    GLFWwindow* nativeWindow = static_cast<GLFWwindow*>(GetWindow().GetNativePointer());

    _guiManager = std::make_unique<Syn::GuiManager>();
    _guiManager->Init(
        nativeWindow,
        vkContext->GetInstance()->Handle(),
        vkContext->GetPhysicalDevice()->Handle(),
        vkContext->GetDevice()->Handle(),
        vkContext->GetDevice()->GetGraphicsQueue()->Handle(),
        vkContext->GetSwapChain()->GetImageCount(),
        vkContext->GetSwapChain()->GetImageFormat()
    );

    using TransformWin = Syn::EditorWindow<Syn::TransformView, Syn::TransformViewModel>;
    _guiManager->AddWindow<TransformWin>(
        Syn::TransformView{
        },
        Syn::TransformViewModel{
            _editorApi.get(),
            _editorApi.get()
        });

    using ViewportWin = Syn::EditorWindow<Syn::ViewportView, Syn::ViewportViewModel>;
    _guiManager->AddWindow<ViewportWin>(
        Syn::ViewportView{},
        Syn::ViewportViewModel{
            _editorApi.get(),
            _editorApi.get(),
            _editorApi.get()
        }
    );

    using SettingsWin = Syn::EditorWindow<Syn::SettingsView, Syn::SettingsViewModel>;
    _guiManager->AddWindow<SettingsWin>(
        Syn::SettingsView{},
        Syn::SettingsViewModel{
            _editorApi.get()
        });
#endif

    _inputDispatcher = std::make_unique<Syn::InputDispatcher>(_guiManager.get(), _engine.get());
}

void Synapse::OnUpdate(float dt) {
#ifndef SYN_PERFORMANCE

    if (_guiManager) {
        _guiManager->BeginFrame();
        _guiManager->UpdateAndDraw();
        _guiManager->EndFrame();
    }

#endif

    if (_engine) {
        _engine->Update(dt);
    }
}

void Synapse::OnRender()
{
    if(_engine) {
        _engine->Render();
	}
}

void Synapse::OnKey(int key, int scancode, int action, int mods) {
    _inputDispatcher->DispatchKey(key, scancode, action, mods);
}

void Synapse::OnMouseButton(int button, int action, int mods) {
    _inputDispatcher->DispatchMouseButton(button, action, mods);
}

void Synapse::OnMouseMove(float x, float y) {
    _inputDispatcher->DispatchMouseMove(x, y);
}

void Synapse::OnResize(uint32_t width, uint32_t height) {
    if (_engine) {
        _engine->WindowResizeEvent(width, height);
    }
}

Syn::Application* Syn::CreateApplication() {
    Syn::ApplicationConfig config;
    config.Name = "SynapseEngine";
    config.Width = 1280;
    config.Height = 720;
    config.Resizable = true;

    return new Synapse(config);
}