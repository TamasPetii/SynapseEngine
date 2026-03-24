#pragma once
#include "Core/Application.h"
#include "Engine/Engine.h"
#include "Manager/GuiManager.h"
#include "Dispatcher/InputDispatcher.h"
#include "EditorApiImpl.h"
#include <memory>

class Synapse : public Syn::Application {
public:
    Synapse(const Syn::ApplicationConfig& config);
    ~Synapse() override;

    void OnInit() override;
    void OnUpdate(float dt) override;
    void OnRender() override;

    void OnKey(int key, int scancode, int action, int mods) override;
    void OnMouseButton(int button, int action, int mods) override;
    void OnMouseMove(float x, float y) override;
    void OnResize(uint32_t width, uint32_t height) override;

private:
    std::unique_ptr<Syn::Engine> _engine;
    std::unique_ptr<Syn::GuiManager> _guiManager;
    std::unique_ptr<Syn::EditorApiImpl> _editorApi;
    std::unique_ptr<Syn::InputDispatcher> _inputDispatcher;
};