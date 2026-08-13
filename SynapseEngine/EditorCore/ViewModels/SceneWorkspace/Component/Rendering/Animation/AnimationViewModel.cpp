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

#include "AnimationViewModel.h"

namespace Syn
{
    AnimationViewModel::AnimationViewModel(ISelectionApi* selectionApi, IAnimationCompApi* animApi)
        : _selectionApi(selectionApi), _animApi(animApi) {}

    const AnimationState& AnimationViewModel::GetState() const { return _state; }

    void AnimationViewModel::SyncWithEngine()
    {
        if (!_selectionApi || !_animApi)
            return;

        EntityID activeEntity = _selectionApi->GetSelectedEntity();

        if (activeEntity != NULL_ENTITY && _animApi->HasAnimation(activeEntity))
        {
            _state.hasComponent = true;

            if (!_speedDrag.IsDragging())
                _state.speed = _animApi->GetAnimationSpeed(activeEntity);

            _state.animationIndex = _animApi->GetAnimationIndex(activeEntity);
            _state.availableAnimations = _animApi->GetAvailableAnimations();
        }
        else
        {
            _state.hasComponent = false;
        }
    }

    void AnimationViewModel::Dispatch(const AnimationIntent& intent)
    {
        EntityID active = _selectionApi->GetSelectedEntity();
        if (active == NULL_ENTITY)
            return;

        std::visit([this, active](auto&& arg)
            {
                using T = std::decay_t<decltype(arg)>;

                if constexpr (std::is_same_v<T, SetAnimationSpeedIntent>) {
                    _speedDrag.Handle(arg.isDragging, arg.speed, _state.speed,
                        [&](const float& v) { _animApi->SetAnimationSpeed(active, v); },
                        [&](const float& s, const float& e) { return std::make_shared<ChangeAnimationSpeedCommand>(_animApi, active, s, e); });
                }
                else if constexpr (std::is_same_v<T, SetAnimationIndexIntent>) {
                    if (_state.animationIndex != arg.animationIndex) {
                        _state.animationIndex = arg.animationIndex;
                        _animApi->SetAnimationIndex(active, arg.animationIndex);
                    }
                }
            }, intent);
    }
}