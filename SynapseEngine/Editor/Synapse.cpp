#include "Synapse.h"
#include "Engine/SynMacro.h"
#include "Engine/Vk/Context.h"
#include <GLFW/glfw3.h>
#include <filesystem>

#include "Editor/View/Component/ComponentView.h"
#include "EditorCore/ViewModels/Component/ComponentViewModel.h"

#include "Editor/View/Viewport/ViewportView.h"
#include "EditorCore/ViewModels/Viewport/ViewportViewModel.h"

#include "Editor/View/Settings/SettingsView.h"
#include "EditorCore/ViewModels/Settings/SettingsViewModel.h"

#include "Editor/View/MainMenu/MainMenuView.h"
#include "EditorCore/ViewModels/MainMenu/MainMenuViewModel.h"

#include "Editor/View/MaterialGraph/MaterialGraphView.h"
#include "EditorCore/ViewModels/MaterialGraph/MaterialGraphViewModel.h"

#include "Editor/View/ContentBrowser/ContentBrowserView.h"
#include "EditorCore/ViewModels/ContentBrowser/ContentBrowserViewModel.h"

#include "Editor/View/Hierarchy/HierarchyView.h"
#include "EditorCore/ViewModels/Hierarchy/HierarchyViewModel.h"

#include "Editor/View/Benchmark/BenchmarkView.h"
#include "EditorCore/ViewModels/Benchmark/BenchmarkViewModel.h"

#include "Manager/GuiTextureManager.h"
#include "Manager/EditorIcons.h"

#include "Engine/Utils/PathUtils.h"

Synapse::Synapse(const Syn::ApplicationConfig& config)
    : Syn::Application(config)
{
}

Synapse::~Synapse() {
    if (_engine && _engine->GetVkContext() && _engine->GetVkContext()->GetDevice()) {
        _engine->GetVkContext()->GetDevice()->WaitIdle();
    }

    _editorApi.reset();
    _iconManager.reset();
    _inputDispatcher.reset();
    _guiManager.reset();
    _engine.reset();
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
        _guiManager->GetTextureManager()->FlushQueue(frameIndex);
        };
#endif

    _engine = std::make_unique<Syn::Engine>(params);

#ifndef SYN_PERFORMANCE

    auto vkContext = _engine->GetVkContext();
    GLFWwindow* nativeWindow = static_cast<GLFWwindow*>(GetWindow().GetNativePointer());

    _guiManager = std::make_unique<Syn::GuiManager>();
    _guiManager->Init(
        nativeWindow,
        vkContext->GetInstance()->Handle(),
        vkContext->GetPhysicalDevice()->Handle(),
        vkContext->GetDevice()->Handle(),
        vkContext->GetDevice()->GetGraphicsQueue()->Handle(),
        vkContext->GetDevice()->GetGraphicsQueue()->GetFamilyIndex(),
        vkContext->GetSwapChain()->GetImageCount(),
        vkContext->GetSwapChain()->GetImageFormat()
    );

    _editorApi = std::make_unique<Syn::EditorApiImpl>(_engine.get(), _guiManager->GetTextureManager());

    _iconManager = std::make_unique<Syn::IconManager>(
        _engine->GetImageManager(),
        _guiManager->GetTextureManager()
    );

    ImGuiIO& io = ImGui::GetIO();
    io.Fonts->AddFontDefault();
    _iconManager->InitializeFontAwesome(io, Syn::PathUtils::GetAbsolutePathString(FONT_PATH), 16.0f);
    _guiManager->CreateFontTexture();
    _iconManager->LoadEngineIcons(Syn::PathUtils::GetAbsolutePathString(ICON_PATH));

    using ComponentWin = Syn::EditorWindow<Syn::ComponentView, Syn::ComponentViewModel>;
    _guiManager->AddWindow<ComponentWin>(
        Syn::ComponentView{
        },
        Syn::ComponentViewModel{
            _editorApi.get(),
            _editorApi.get(),
            _editorApi.get(),
        });

    using ViewportWin = Syn::EditorWindow<Syn::ViewportView, Syn::ViewportViewModel>;
    _guiManager->AddWindow<ViewportWin>(
        Syn::ViewportView{},
        Syn::ViewportViewModel{
            _editorApi.get(),
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

    using MainMenuWin = Syn::EditorWindow<Syn::MainMenuView, Syn::MainMenuViewModel>;
    _guiManager->AddWindow<MainMenuWin>(
        Syn::MainMenuView{},
        Syn::MainMenuViewModel{
            _editorApi.get(),
			_guiManager->GetFileDialog()
        }
    );

    using MaterialGraphWin = Syn::EditorWindow<Syn::MaterialGraphView, Syn::MaterialGraphViewModel>;
    _guiManager->AddWindow<MaterialGraphWin>(
        Syn::MaterialGraphView{},
        Syn::MaterialGraphViewModel{
            _editorApi.get()
        }
    );

    std::string absoluteAssetsPath = std::filesystem::absolute(ASSET_PATH).generic_string();

    using ContentBrowserWin = Syn::EditorWindow<Syn::ContentBrowserView, Syn::ContentBrowserViewModel>;
    _guiManager->AddWindow<ContentBrowserWin>(
        Syn::ContentBrowserView{ _iconManager.get() },
        Syn::ContentBrowserViewModel{ _editorApi.get(), absoluteAssetsPath }
    );

    using HierarchyWin = Syn::EditorWindow<Syn::HierarchyView, Syn::HierarchyViewModel>;
    _guiManager->AddWindow<HierarchyWin>(
        Syn::HierarchyView{},
        Syn::HierarchyViewModel{
            _editorApi.get(),
            _editorApi.get(),
            _editorApi.get(),
        }
    );

    using BenchmarkWin = Syn::EditorWindow<Syn::BenchmarkView, Syn::BenchmarkViewModel>;
    _guiManager->AddWindow<BenchmarkWin>(
        Syn::BenchmarkView{},
        Syn::BenchmarkViewModel{}
    );

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

void Synapse::OnScroll(float xOffset, float yOffset) {
    _inputDispatcher->DispatchScroll(xOffset, yOffset);
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