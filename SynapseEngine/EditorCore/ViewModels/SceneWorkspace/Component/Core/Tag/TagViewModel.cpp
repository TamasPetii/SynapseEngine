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

#include "TagViewModel.h"
#include "EditorCore/Types/EntityHandle.h"

namespace Syn {

    TagViewModel::TagViewModel(ISelectionApi* selectionApi, ITagApi* tagApi)
        : _selectionApi(selectionApi), _tagApi(tagApi)
    {}

    const TagState& TagViewModel::GetState() const {
        return _state;
    }

    void TagViewModel::SyncWithEngine() {
        if (!_selectionApi || !_tagApi) return;

        EntityID activeEntity = _selectionApi->GetSelectedEntity();

        if (activeEntity != NULL_ENTITY) {
            _state.name = _tagApi->GetEntityName(activeEntity);
            _state.tag = _tagApi->GetEntityTag(activeEntity);
            _state.isEnabled = _tagApi->IsEntityEnabled(activeEntity);
        }
    }

    void TagViewModel::Dispatch(const TagIntent& intent) {
        std::visit([this](auto&& arg) {
            using T = std::decay_t<decltype(arg)>;

            if constexpr (std::is_same_v<T, SetEntityNameIntent>) {
                _state.name = arg.newName;

                if (_selectionApi && _tagApi) {
                    EntityID activeEntity = _selectionApi->GetSelectedEntity();
                    if (activeEntity != NULL_ENTITY) {
                        _tagApi->SetEntityName(activeEntity, arg.newName);
                    }
                }
            }
            else if constexpr (std::is_same_v<T, SetEntityTagIntent>) {
                _state.tag = arg.newTag;

                if (_selectionApi && _tagApi) {
                    EntityID activeEntity = _selectionApi->GetSelectedEntity();
                    if (activeEntity != NULL_ENTITY) {
                        _tagApi->SetEntityTag(activeEntity, arg.newTag);
                    }
                }
            }
            else if constexpr (std::is_same_v<T, ToggleEntityIntent>) {
                _state.isEnabled = arg.isEnabled;

                if (_selectionApi && _tagApi) {
                    EntityID activeEntity = _selectionApi->GetSelectedEntity();
                    if (activeEntity != NULL_ENTITY) {
                        _tagApi->SetEntityEnabled(activeEntity, arg.isEnabled);
                    }
                }
            }
            }, intent);
    }
}