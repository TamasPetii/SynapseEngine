// Copyright (C) 2026 Tamás Péter
// This file is part of SynapseEngine.
//
// SynapseEngine is free software: you can redistribute it and/or modify
// it under the terms of the GNU Affero General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// SynapseEngine is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License
// along with SynapseEngine. If not, see <https://www.gnu.org/licenses/>.

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

    void InputDispatcher::DispatchScroll(float xOffset, float yOffset) {
        if (_gui)
            _gui->OnScroll(xOffset, yOffset);

        if (_engine && !IsGuiCapturingMouse()) {
            _engine->OnScroll(xOffset, yOffset);
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

    void InputDispatcher::DispatchChar(unsigned int codepoint) {
        if (_gui)
            _gui->OnChar(codepoint);

        if (_engine && !IsGuiCapturingKeyboard()) {
            _engine->OnChar(codepoint);
        }
    }
}