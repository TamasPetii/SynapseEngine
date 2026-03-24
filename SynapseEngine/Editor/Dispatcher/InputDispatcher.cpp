#include "InputDispatcher.h"
#include "Engine/Engine.h"
#include "Editor/Manager/GuiManager.h"

namespace Syn {

    void InputDispatcher::DispatchMouseMove(float x, float y) {
        if (_gui)
            _gui->OnMouseMove(x, y);

        if (_engine && !IsGuiCapturingMouse()) {
            _engine->OnMouseMove(x, y);
        }
    }

    void InputDispatcher::DispatchMouseButton(int button, int action, int mods) {
        if (_gui)
            _gui->OnMouseButton(button, action, mods);

        if (_engine && !IsGuiCapturingMouse()) {
            _engine->OnMouseButton(button, action, mods);
        }
    }

    void InputDispatcher::DispatchKey(int key, int scancode, int action, int mods) {
        if (_gui)
            _gui->OnKey(key, scancode, action, mods);

        if (_engine && !IsGuiCapturingKeyboard()) {
            _engine->OnKey(key, scancode, action, mods);
        }
    }

    bool InputDispatcher::IsGuiCapturingMouse() const {
        return false;
        return _gui && _gui->WantsCaptureMouse();
    }

    bool InputDispatcher::IsGuiCapturingKeyboard() const {
        return false;
        return _gui && _gui->WantsCaptureKeyboard();
    }
}