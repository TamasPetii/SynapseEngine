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
    //ImGui_ImplGlfw_KeyCallback(window, key, scancode, action, mods);

    if(_engine)
        _engine->OnKey(key, scancode, action, mods);
}

void Synapse::OnMouseButton(int button, int action, int mods) {
    //ImGui_ImplGlfw_MouseButtonCallback(window, button, action, mods);

    if (_engine)
        _engine->OnMouseButton(button, action, mods);
}

void Synapse::OnMouseMove(float x, float y) {  
    //ImGui_ImplGlfw_CursorPosCallback(window, xpos, ypos);

    if (_engine)
        _engine->OnMouseMove(x, y);
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