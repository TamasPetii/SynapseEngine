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
#include "EditorCore/ViewModels/IViewModel.h"
#include "TextureGraphState.h"
#include "TextureGraphIntent.h"
#include "EditorCore/Api/ITextureApi.h"

namespace Syn {
    class TextureGraphViewModel : public IViewModel<TextureGraphState, TextureGraphIntent> {
    public:
        TextureGraphViewModel(ITextureApi* textureApi);
        ~TextureGraphViewModel() override = default;

        const TextureGraphState& GetState() const override { return _state; }

        void SyncWithEngine() override;
        void Dispatch(const TextureGraphIntent& intent) override;
    private:
        ITextureApi* _textureApi = nullptr;
        uint32_t _lastSelectedId = 0xFFFFFFFF;
        TextureGraphState _state;
    };
}