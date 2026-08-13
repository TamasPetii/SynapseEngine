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
#include "EditorCore/Command/CommandRegistry.h"
#include <memory>

namespace Syn 
{
    template <typename T>
    class DragInteraction {
    public:
        template <typename LiveUpdateFunc, typename CreateCommandFunc>
        void Handle(bool intentIsDragging, const T& newValue, T& stateRef, LiveUpdateFunc liveUpdate, CreateCommandFunc createCommand)
        {
            if (intentIsDragging && !_isDragging) {
                _startValue = stateRef;
                _isDragging = true;
            }

            stateRef = newValue;

            if (intentIsDragging) {
                liveUpdate(newValue);
            }
            else {
                _isDragging = false;

                auto cmd = createCommand(_startValue, newValue);

                if (cmd) {
                    CommandRegistry::Get().ExecuteCommand(cmd);
                }
            }
        }

        bool IsDragging() const { return _isDragging; }
    private:
        bool _isDragging = false;
        T _startValue{};
    };
}