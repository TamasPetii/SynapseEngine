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
#include "MaterialGraphState.h"
#include "MaterialGraphIntent.h"
#include "EditorCore/Api/IMaterialApi.h"
#include "EditorCore/Api/ITextureApi.h"

namespace Syn {
    class MaterialGraphViewModel : public IViewModel<MaterialGraphState, MaterialGraphIntent> {
    public:
        MaterialGraphViewModel(IMaterialApi* materialApi, ITextureApi* textureApi);

        const MaterialGraphState& GetState() const override { return _state; }

        void SyncWithEngine() override;
        void Dispatch(const MaterialGraphIntent& intent) override;
    private:
        void RebuildGraphForSelectedMaterial(uint32_t materialId);
        void HandleCreateLink(const CreateLinkIntent& intent);
        void HandleDeleteLink(const DeleteLinkIntent& intent);
    private:
        IMaterialApi* _materialApi = nullptr;
        ITextureApi* _textureApi = nullptr;

        MaterialGraphState _state;

        uint32_t _lastSelectedMaterial = INVALID_MATERIAL_ID;
        uint64_t _lastEngineVersion = 0;
    };
}