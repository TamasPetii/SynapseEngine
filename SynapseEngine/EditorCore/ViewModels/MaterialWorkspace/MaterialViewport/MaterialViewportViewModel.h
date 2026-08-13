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
#include "MaterialViewportState.h"
#include "MaterialViewportIntent.h"
#include "EditorCore/Api/IRenderApi.h"

namespace Syn {
    class MaterialViewportViewModel : public IViewModel<MaterialViewportState, MaterialViewportIntent> {
    public:
        explicit MaterialViewportViewModel(IRenderApi* renderApi);
        ~MaterialViewportViewModel() override = default;

        const MaterialViewportState& GetState() const override;

        void SyncWithEngine() override;
        void Dispatch(const MaterialViewportIntent& intent) override;

    private:
        void HandleResize(const ResizeMaterialViewportIntent& intent);

    private:
        IRenderApi* _renderApi = nullptr;
        MaterialViewportState _state;
    };
}