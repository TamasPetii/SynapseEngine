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

#include "MaterialOverrideViewModel.h"

namespace Syn
{
    MaterialOverrideViewModel::MaterialOverrideViewModel(ISelectionApi* selectionApi, IMaterialOverrideApi* overrideApi)
        : _selectionApi(selectionApi), _overrideApi(overrideApi) {}

    const MaterialOverrideState& MaterialOverrideViewModel::GetState() const { return _state; }

    void MaterialOverrideViewModel::SyncWithEngine()
    {
        if (!_selectionApi || !_overrideApi)
            return;

        EntityID activeEntity = _selectionApi->GetSelectedEntity();

        if (activeEntity != NULL_ENTITY && _overrideApi->HasMaterialOverride(activeEntity))
        {
            _state.hasComponent = true;

            _state.expectedSlotCount = _overrideApi->GetExpectedSlotCount(activeEntity);
            _state.sharedMaterialEntity = _overrideApi->GetSharedMaterialEntity(activeEntity);
            _state.availableMaterials = _overrideApi->GetAvailableMaterials();
            //_state.compatibleSharedEntities = _overrideApi->GetCompatibleSharedEntities(activeEntity);

            _state.overrides.clear();
            _state.overrides.reserve(_state.expectedSlotCount);
            for (uint32_t i = 0; i < _state.expectedSlotCount; ++i) {
                _state.overrides.push_back(_overrideApi->GetMaterialAtSlot(activeEntity, i));
            }
        }
        else
        {
            _state.hasComponent = false;
        }
    }

    void MaterialOverrideViewModel::Dispatch(const MaterialOverrideIntent& intent)
    {
        EntityID active = _selectionApi->GetSelectedEntity();
        if (active == NULL_ENTITY)
            return;

        std::visit([this, active](auto&& arg)
            {
                using T = std::decay_t<decltype(arg)>;

                if constexpr (std::is_same_v<T, SetMaterialOverrideSlotIntent>) {
                    if (arg.slotIndex < _state.overrides.size()) {
                        _state.overrides[arg.slotIndex] = arg.materialId;
                        _overrideApi->SetMaterialAtSlot(active, arg.slotIndex, arg.materialId);
                    }
                }
                else if constexpr (std::is_same_v<T, SetSharedMaterialEntityIntent>) {
                    _state.sharedMaterialEntity = arg.sharedEntity;
                    _overrideApi->SetSharedMaterialEntity(active, arg.sharedEntity);
                }
            }, intent);
    }
}