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
        void DispatchScroll(float xOffset, float yOffset);
        void DispatchChar(unsigned int codepoint);
    private:
        bool IsGuiCapturingMouse() const;
        bool IsGuiCapturingKeyboard() const;
    private:
        GuiManager* _gui;
        Engine* _engine;
    };
}