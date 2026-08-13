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
#include <vector>
#include <memory>
#include "Editor/Workspace/IGuiWindow.h"

namespace Syn 
{
    enum class EditorWorkspace {
        Scene,
        Model,
        Material,
        Texture,
        Animation,
        Audio
    };

    class IWorkspace {
    public:
        virtual ~IWorkspace() = default;

        virtual void Initialize() = 0;
        virtual void OnActivate() {}
        virtual void OnDeactivate() {}

        virtual void UpdateAndDraw() {
            for (auto& window : _windows) {
                window->UpdateAndDraw();
            }
        }

    protected:

        template<typename TWindow, typename... Args>
        void AddWindow(Args&&... args) {
            _windows.push_back(std::make_unique<TWindow>(std::forward<Args>(args)...));
        }

        std::vector<std::unique_ptr<IGuiWindow>> _windows;
    };
}