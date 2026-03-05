#include "Synapse.h"
#include "Engine/SynMacro.h"
#include <print>

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

    params.onRenderGuiCallback = [&]() {
        /*
        if (_gui) {
            // _gui->Render(cmd, registry, resMgr, frame);
        }
        */
    };

    _engine = std::make_shared<Syn::Engine>(params);
    std::println("Sandbox Initialized successfully!");
}

void Synapse::OnUpdate(float dt) {
    if (_engine) {
        _engine->Update();
    }
}

void Synapse::OnRender()
{
    if(_engine) {
        _engine->Render();
	}
}

void Synapse::OnKey(int key, int scancode, int action, int mods) {
    /*
    ImGui_ImplGlfw_KeyCallback(window, key, scancode, action, mods);

    if (action == GLFW_PRESS) {
        InputManager::Instance()->SetKeyboardKey(key, true);
    }
    else if (action == GLFW_RELEASE) {
        InputManager::Instance()->SetKeyboardKey(key, false);
    }
    */
}

void Synapse::OnMouseButton(int button, int action, int mods) {
    /*
    ImGui_ImplGlfw_MouseButtonCallback(window, button, action, mods);

    if (action == GLFW_PRESS)
    {
        InputManager::Instance()->SetMouseButton(button, true);
    }
    else if (action == GLFW_RELEASE)
    {
        InputManager::Instance()->SetMouseButton(button, false);
    }
    */
}

void Synapse::OnMouseMove(float x, float y) {
    /*
    ImGui_ImplGlfw_CursorPosCallback(window, xpos, ypos);

    InputManager::Instance()->SetMousePosition(xpos, ypos);
    */
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