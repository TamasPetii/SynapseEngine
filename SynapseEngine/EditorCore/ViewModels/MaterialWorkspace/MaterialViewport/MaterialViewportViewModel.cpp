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

#include "MaterialViewportViewModel.h"

namespace Syn {

    MaterialViewportViewModel::MaterialViewportViewModel(IRenderApi* renderApi)
        : _renderApi(renderApi) {}

    const MaterialViewportState& MaterialViewportViewModel::GetState() const {
        return _state;
    }

    void MaterialViewportViewModel::SyncWithEngine() {
        if (!_renderApi) return;

        _state.textureId = _renderApi->GetViewportTexture(
            _state.currentGroup,
            _state.currentTarget,
            _state.currentView
        );
    }

    void MaterialViewportViewModel::Dispatch(const MaterialViewportIntent& intent) {
        std::visit([this](auto&& arg) {
            using T = std::decay_t<decltype(arg)>;
            if constexpr (std::is_same_v<T, ResizeMaterialViewportIntent>) {
                HandleResize(arg);
            }
            }, intent);
    }

    void MaterialViewportViewModel::HandleResize(const ResizeMaterialViewportIntent& intent) {
        if (intent.width > 0 && intent.height > 0 &&
            (_state.width != intent.width || _state.height != intent.height))
        {
            _state.width = intent.width;
            _state.height = intent.height;

            _renderApi->ResizeRenderTargets(_state.width, _state.height);
        }
    }

}