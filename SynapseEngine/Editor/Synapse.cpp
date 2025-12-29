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
    /*
    _engine = std::make_shared<Engine>();

    _engine->SetRequiredWindowExtensions(GetWindow().GetRequiredExtensions());

    _engine->SetSurfaceCreationFunction(
        [this](const Vk::Instance* const instance, VkSurfaceKHR* surface) -> void {
            *surface = GetWindow().CreateSurface(instance->Value());
        }
    );

    _engine->SetWindowExtentFunction(
        [this]() -> std::pair<int, int> {
            auto [width, height] = GetWindow().GetSize();
            return std::make_pair((int)width, (int)height);
        }
    );

    _engine->Initialize();

    _engine->SetGuiRenderFunction(
        [this](VkCommandBuffer cmd, auto registry, auto resMgr, uint32_t frame) {
            _gui->Render(cmd, registry, resMgr, frame);
        }
    );
    */

    std::println("Sandbox Initialized successfully!");
}

void Synapse::OnUpdate(float dt) {
    /*
    if (_engine) {
        _engine->SimulateFrame();
    }
    */
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
    /*
    if (_engine) {
        _engine->WindowResizeEvent();
    }
    */
}

Syn::Application* Syn::CreateApplication() {
    Syn::ApplicationConfig config;
    config.Name = "SynapseEngine";
    config.Width = 1280;
    config.Height = 720;
    config.Resizable = true;

    return new Synapse(config);
}