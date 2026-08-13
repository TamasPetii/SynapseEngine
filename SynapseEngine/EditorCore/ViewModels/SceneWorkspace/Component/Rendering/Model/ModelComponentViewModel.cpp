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

#include "ModelComponentViewModel.h"

namespace Syn
{
    ModelComponentViewModel::ModelComponentViewModel(ISelectionApi* selectionApi, IModelComponentApi* modelApi)
        : _selectionApi(selectionApi), _modelApi(modelApi) {}

    const ModelComponentState& ModelComponentViewModel::GetState() const { return _state; }

    void ModelComponentViewModel::SyncWithEngine()
    {
        if (!_selectionApi || !_modelApi)
            return;

        EntityID activeEntity = _selectionApi->GetSelectedEntity();

        if (activeEntity != NULL_ENTITY && _modelApi->HasModelComponent(activeEntity))
        {
            _state.hasComponent = true;

            _state.castShadow = _modelApi->GetCastShadow(activeEntity);
            _state.receiveShadow = _modelApi->GetReceiveShadow(activeEntity);
            _state.modelIndex = _modelApi->GetModelIndex(activeEntity);

            _state.availableModels = _modelApi->GetAvailableModels();
        }
        else
        {
            _state.hasComponent = false;
        }
    }

    void ModelComponentViewModel::Dispatch(const ModelComponentIntent& intent)
    {
        EntityID active = _selectionApi->GetSelectedEntity();
        if (active == NULL_ENTITY)
            return;

        std::visit([this, active](auto&& arg)
            {
                using T = std::decay_t<decltype(arg)>;

                if constexpr (std::is_same_v<T, SetModelCastShadowIntent>) {
                    _state.castShadow = arg.castShadow;
                    _modelApi->SetCastShadow(active, arg.castShadow);
                }
                else if constexpr (std::is_same_v<T, SetModelReceiveShadowIntent>) {
                    _state.receiveShadow = arg.receiveShadow;
                    _modelApi->SetReceiveShadow(active, arg.receiveShadow);
                }
                else if constexpr (std::is_same_v<T, SetModelIndexIntent>) {
                    if (_state.modelIndex != arg.modelIndex) {
                        _state.modelIndex = arg.modelIndex;
                        _modelApi->SetModelIndex(active, arg.modelIndex);
                    }
                }
            }, intent);
    }
}