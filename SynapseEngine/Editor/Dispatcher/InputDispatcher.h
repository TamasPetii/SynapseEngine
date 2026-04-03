#pragma once
#include <functional>
#include <vector>

namespace Syn {

    class Engine;
    class GuiManager;

    class InputDispatcher {
    public:
        InputDispatcher(GuiManager* gui, Engine* engine) : _gui(gui), _engine(engine) {}

        void DispatchMouseMove(float x, float y);
        void DispatchMouseButton(int button, int action, int mods);
        void DispatchKey(int key, int scancode, int action, int mods);
    private:
        bool IsGuiCapturingMouse() const;
        bool IsGuiCapturingKeyboard() const;
    private:
        GuiManager* _gui;
        Engine* _engine;
    };
}