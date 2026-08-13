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

#include "TransformViewModel.h"
#include "EditorCore/Types/EntityHandle.h" // A NULL_ENTITY miatt

namespace Syn {

    TransformViewModel::TransformViewModel(ISelectionApi* selectionApi, ITransformApi* transformApi, IHierarchyApi* hierarchyApi)
        : _selectionApi(selectionApi), _transformApi(transformApi), _hierarchyApi(hierarchyApi)
    {}

    const TransformState& TransformViewModel::GetState() const {
        return _state;
    }

    void TransformViewModel::SyncWithEngine() {
        if (!_selectionApi || !_transformApi) return;

        EntityID activeEntity = _selectionApi->GetSelectedEntity();

        if (activeEntity != NULL_ENTITY) {
            if (!_positionDrag.IsDragging()) _state.position = _transformApi->GetEntityPosition(activeEntity);
            if (!_rotationDrag.IsDragging()) _state.rotation = _transformApi->GetEntityRotation(activeEntity);
            if (!_scaleDrag.IsDragging())    _state.scale = _transformApi->GetEntityScale(activeEntity);
        }
    }

    void TransformViewModel::Dispatch(const TransformIntent& intent) {
        std::visit([this](auto&& arg) {
            using T = std::decay_t<decltype(arg)>;

            if constexpr (std::is_same_v<T, SetPositionIntent>)
                HandleSetPosition(arg);
            else if constexpr (std::is_same_v<T, SetRotationIntent>)
                HandleSetRotation(arg);
            else if constexpr (std::is_same_v<T, SetScaleIntent>)
                HandleSetScale(arg);
            }, intent);
    }

    void TransformViewModel::HandleSetPosition(const SetPositionIntent& intent) {
        EntityID activeEntity = _selectionApi->GetSelectedEntity();
        if (activeEntity == NULL_ENTITY) return;

        _positionDrag.Handle(
            intent.isDragging, intent.newPosition, _state.position,

            [&](const glm::vec3& pos) {
                _transformApi->SetEntityPosition(activeEntity, pos);
            },

            [&](const glm::vec3& start, const glm::vec3& end) {
                return std::make_shared<ChangePositionCommand>(_transformApi, activeEntity, start, end);
            }
        );
    }

    void TransformViewModel::HandleSetRotation(const SetRotationIntent& intent) {
        EntityID activeEntity = _selectionApi->GetSelectedEntity();
        if (activeEntity == NULL_ENTITY) return;

        _rotationDrag.Handle(
            intent.isDragging, intent.newRotation, _state.rotation,

            [&](const glm::vec3& rot) {
                _transformApi->SetEntityRotation(activeEntity, rot);
            },

            [&](const glm::vec3& start, const glm::vec3& end) {
                return std::make_shared<ChangeRotationCommand>(_transformApi, activeEntity, start, end);
            }
        );
    }

    void TransformViewModel::HandleSetScale(const SetScaleIntent& intent) {
        EntityID activeEntity = _selectionApi->GetSelectedEntity();
        if (activeEntity == NULL_ENTITY) return;

        _scaleDrag.Handle(
            intent.isDragging, intent.newScale, _state.scale,

            [&](const glm::vec3& scl) {
                _transformApi->SetEntityScale(activeEntity, scl);
            },

            [&](const glm::vec3& start, const glm::vec3& end) {
                return std::make_shared<ChangeScaleCommand>(_transformApi, activeEntity, start, end);
            }
        );
    }

}