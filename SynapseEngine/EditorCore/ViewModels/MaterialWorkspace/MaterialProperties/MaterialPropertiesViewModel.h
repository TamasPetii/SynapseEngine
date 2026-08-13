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
#include "MaterialPropertiesState.h"
#include "MaterialPropertiesIntent.h"
#include "EditorCore/Api/IMaterialApi.h"
#include "EditorCore/Api/ITextureApi.h"

namespace Syn {
    class MaterialPropertiesViewModel : public IViewModel<MaterialPropertiesState, MaterialPropertiesIntent> {
    public:
        MaterialPropertiesViewModel(IMaterialApi* materialApi, ITextureApi* textureApi);
        ~MaterialPropertiesViewModel() override = default;

        const MaterialPropertiesState& GetState() const override { return _state; }
        void SyncWithEngine() override;
        void Dispatch(const MaterialPropertiesIntent& intent) override;
    private:
        IMaterialApi* _materialApi = nullptr;
        ITextureApi* _textureApi = nullptr;
        MaterialPropertiesState _state;
    };
}