#pragma once
#include "Core/Application.h"
#include "Engine/Engine.h"
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
    std::shared_ptr<Syn::Engine> _engine;
    // std::shared_ptr<Gui> _gui;
};