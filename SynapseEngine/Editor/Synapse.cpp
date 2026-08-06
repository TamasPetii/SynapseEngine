#include "Synapse.h"
#include "Engine/SynMacro.h"
#include "Engine/Vk/Context.h"
#include <GLFW/glfw3.h>
#include <filesystem>

#include "Editor/Workspace/SceneWorkspace/SceneWorkspace.h"
#include "Editor/Workspace/ModelWorkspace/ModelWorkspace.h"
#include "Editor/Workspace/MaterialWorkspace/MaterialWorkspace.h"
#include "Editor/Workspace/TextureWorkspace/TextureWorkspace.h"
#include "Editor/Workspace/AnimationWorkspace/AnimationWorkspace.h"

#include "Editor/Workspace/Common/MainMenu/MainMenuView.h"
#include "EditorCore/ViewModels/Common/MainMenu/MainMenuViewModel.h"

#include "Manager/GuiTextureManager.h"
#include "Manager/EditorIcons.h"
#include "Engine/Utils/PathUtils.h"
#include "Editor/Workspace/IGuiWindow.h"

#include "Engine/Image/Loader/SvgImageLoader.h"
#include "Engine/Logger/SynLog.h"

Synapse::Synapse(const Syn::ApplicationConfig& config)
    : Syn::Application(config)
{
}

Synapse::~Synapse() {
    if (_engine && _engine->GetVkContext() && _engine->GetVkContext()->GetDevice()) {
        _engine->GetVkContext()->GetDevice()->WaitIdle();
    }

    _editorContext.reset();
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

    std::string iconPath = Syn::PathUtils::GetAbsolutePathString(std::string(ASSET_PATH) + "/dark_icon.svg");
    Syn::SvgImageLoader svgLoader;
    if (auto rawImageOpt = svgLoader.LoadFile(iconPath)) {
        const auto& rawImage = rawImageOpt.value();
        GetWindow().SetIcon(rawImage.width, rawImage.height, rawImage.pixels.data());
    }
    else {
        Syn::Error("Failed to load window icon from: {}", iconPath);
    }

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

    _editorContext = std::make_unique<Syn::EditorContext>(_engine.get(), _guiManager->GetTextureManager());

    _iconManager = std::make_unique<Syn::IconManager>(
        _engine->GetImageManager(),
        _guiManager->GetTextureManager()
    );

    ImGuiIO& io = ImGui::GetIO();
    ImFontConfig defaultConfig;
    defaultConfig.SizePixels = 13.0f;
    io.Fonts->AddFontDefault(&defaultConfig);

    _iconManager->InitializeFontAwesome(io, Syn::PathUtils::GetAbsolutePathString(FONT_PATH), 16.0f);
    _iconManager->LoadEngineIcons(Syn::PathUtils::GetAbsolutePathString(ICON_PATH));

    std::string absoluteAssetsPath = std::filesystem::absolute(ASSET_PATH).generic_string();

    using MainMenuWin = Syn::EditorWindow<Syn::MainMenuView, Syn::MainMenuViewModel>;
    _guiManager->AddGlobalWindow<MainMenuWin>(
        Syn::MainMenuView{},
        Syn::MainMenuViewModel{ _editorContext->GetApi<Syn::ISceneApi>(), _guiManager->GetFileDialog() }
    );

    _guiManager->AddWorkspace(Syn::EditorWorkspace::Scene, std::make_unique<Syn::SceneWorkspace>(
        _editorContext.get(), _iconManager.get(), absoluteAssetsPath
    ));

    _guiManager->AddWorkspace(Syn::EditorWorkspace::Model, std::make_unique<Syn::ModelWorkspace>(
        _editorContext.get(), _iconManager.get(), absoluteAssetsPath
    ));

    _guiManager->AddWorkspace(Syn::EditorWorkspace::Material, std::make_unique<Syn::MaterialWorkspace>(
        _editorContext.get(), _iconManager.get(), absoluteAssetsPath
    ));

    _guiManager->AddWorkspace(Syn::EditorWorkspace::Texture, std::make_unique<Syn::TextureWorkspace>(
        _editorContext.get(), _iconManager.get(), absoluteAssetsPath
    ));

    _guiManager->AddWorkspace(Syn::EditorWorkspace::Animation, std::make_unique<Syn::AnimationWorkspace>(
        _editorContext.get(), _iconManager.get(), absoluteAssetsPath
    ));

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

void Synapse::OnChar(unsigned int codepoint) {
    _inputDispatcher->DispatchChar(codepoint);
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