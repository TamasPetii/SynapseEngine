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
#include <memory>

namespace Syn 
{
    class IGuiWindow {
    public:
        virtual ~IGuiWindow() = default;
        virtual void UpdateAndDraw() = 0;
    };

    template <typename TView, typename TViewModel>
    class EditorWindow : public IGuiWindow {
    public:
        EditorWindow(TView view, TViewModel viewModel)
            : _view(std::move(view)), _viewModel(std::move(viewModel)) {}

        void UpdateAndDraw() override {
            _viewModel.SyncWithEngine();
            _view.Draw(_viewModel);
        }
    private:
        TView _view;
        TViewModel _viewModel;
    };
}