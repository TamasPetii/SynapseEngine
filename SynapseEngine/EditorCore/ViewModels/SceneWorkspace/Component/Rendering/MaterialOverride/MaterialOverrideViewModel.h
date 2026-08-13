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
#include "MaterialOverrideState.h"
#include "MaterialOverrideIntent.h"
#include "EditorCore/Api/ISelectionApi.h"
#include "EditorCore/Api/IMaterialOverrideApi.h"

namespace Syn {
    class MaterialOverrideViewModel : public IViewModel<MaterialOverrideState, MaterialOverrideIntent> {
    public:
        MaterialOverrideViewModel(ISelectionApi* selectionApi, IMaterialOverrideApi* overrideApi);
        ~MaterialOverrideViewModel() override = default;

        const MaterialOverrideState& GetState() const override;
        void SyncWithEngine() override;
        void Dispatch(const MaterialOverrideIntent& intent) override;

    private:
        ISelectionApi* _selectionApi = nullptr;
        IMaterialOverrideApi* _overrideApi = nullptr;
        MaterialOverrideState _state;
    };
}