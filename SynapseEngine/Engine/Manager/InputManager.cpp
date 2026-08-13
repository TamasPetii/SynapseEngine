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

#include "InputManager.h"

namespace Syn
{
    void InputManager::UpdatePrevious()
    {
        for (auto& [key, state] : _keyStatus)
        {
            state.previous = state.current;
        }

        for (auto& [button, state] : _buttonStatus)
        {
            state.previous = state.current;
        }

        _lastMouseX = _mouseX;
        _lastMouseY = _mouseY;
    }

    void InputManager::SetKeyboardKey(int key, bool active)
    {
        _keyStatus[key].current = active;
    }

    bool InputManager::IsKeyPressed(int key) const
    {
        auto it = _keyStatus.find(key);
        if (it != _keyStatus.end())
        {
            return it->second.current && !it->second.previous;
        }
        return false;
    }

    bool InputManager::IsKeyHeld(int key) const
    {
        auto it = _keyStatus.find(key);
        if (it != _keyStatus.end())
        {
            return it->second.current;
        }
        return false;
    }

    bool InputManager::IsKeyReleased(int key) const
    {
        auto it = _keyStatus.find(key);
        if (it != _keyStatus.end())
        {
            return !it->second.current && it->second.previous;
        }
        return false;
    }

    void InputManager::SetMouseButton(int button, bool active)
    {
        _buttonStatus[button].current = active;
    }

    bool InputManager::IsButtonPressed(int key) const
    {
        auto it = _buttonStatus.find(key);
        if (it != _buttonStatus.end())
        {
            return it->second.current && !it->second.previous;
        }
        return false;
    }

    bool InputManager::IsButtonHeld(int key) const
    {
        auto it = _buttonStatus.find(key);
        if (it != _buttonStatus.end())
        {
            return it->second.current;
        }
        return false;
    }

    bool InputManager::IsButtonReleased(int key) const
    {
        auto it = _buttonStatus.find(key);
        if (it != _buttonStatus.end())
        {
            return !it->second.current && it->second.previous;
        }
        return false;
    }

    void InputManager::SetMousePosition(double x, double y)
    {
        _mouseX = x;
        _mouseY = y;
    }

    std::pair<double, double> InputManager::GetMouseDelta() const
    {
        return { _mouseX - _lastMouseX, _mouseY - _lastMouseY };
    }

    std::pair<double, double> InputManager::GetMousePosition() const
    {
        return { _mouseX, _mouseY };
    }
}