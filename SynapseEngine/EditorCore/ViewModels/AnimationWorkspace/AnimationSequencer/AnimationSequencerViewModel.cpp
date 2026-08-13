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

#include "AnimationSequencerViewModel.h"

namespace Syn {

    AnimationSequencerViewModel::AnimationSequencerViewModel(IAnimationApi* animationApi)
        : _animationApi(animationApi) {}

    const AnimationSequencerState& AnimationSequencerViewModel::GetState() const {
        return _state;
    }

    void AnimationSequencerViewModel::SyncWithEngine() {
        if (!_animationApi) return;

        uint32_t newSelectedId = _animationApi->GetSelected();

        if (_state.activeAnimationId != newSelectedId) {
            _state.activeAnimationId = newSelectedId;

            if (_state.activeAnimationId != 0xFFFFFFFF) {
                _state.currentAnimData = _animationApi->GetAnimationCpuData(_state.activeAnimationId);

                if (_state.currentAnimData) {
                    _state.editableTracks = _state.currentAnimData->tracks;

                    _state.trackUIStates.clear();
                    _state.trackUIStates.reserve(_state.editableTracks.size());

                    float sampleRate = _state.currentAnimData->descriptor.sampleRate;

                    for (const auto& track : _state.editableTracks) {
                        SequencerTrackUIState uiState;
                        uiState.groupOpen = true;

                        for (const auto& key : track.positions)
                            uiState.positionFrames.push_back(static_cast<int32_t>(key.time * sampleRate));

                        for (const auto& key : track.rotations)
                            uiState.rotationFrames.push_back(static_cast<int32_t>(key.time * sampleRate));

                        for (const auto& key : track.scales)
                            uiState.scaleFrames.push_back(static_cast<int32_t>(key.time * sampleRate));

                        _state.trackUIStates.push_back(std::move(uiState));
                    }
                }
            }
            else {
                _state.currentAnimData = nullptr;
                _state.editableTracks.clear();
                _state.trackUIStates.clear();
            }
        }
    }

    void AnimationSequencerViewModel::Dispatch(const AnimationSequencerIntent& intent) {
        std::visit([this](auto&& arg) {
            using T = std::decay_t<decltype(arg)>;

            if constexpr (std::is_same_v<T, ChangeSequencerFrameIntent>) {
                _state.currentFrame = arg.frame;
            }
            else if constexpr (std::is_same_v<T, SelectSequencerTrackIntent>) {
                _state.selectedTrackIndex = arg.trackIndex;
            }
            else if constexpr (std::is_same_v<T, ToggleSequencerGroupIntent>) {
                if (arg.trackIndex < _state.trackUIStates.size()) {
                    _state.trackUIStates[arg.trackIndex].groupOpen = arg.isOpen;
                }
            }
            else if constexpr (std::is_same_v<T, UpdateTrackKeysIntent>) {
                if (arg.trackIndex < _state.trackUIStates.size()) {
                    _state.trackUIStates[arg.trackIndex].positionFrames = arg.positionFrames;
                    _state.trackUIStates[arg.trackIndex].rotationFrames = arg.rotationFrames;
                    _state.trackUIStates[arg.trackIndex].scaleFrames = arg.scaleFrames;
                }
            }
            }, intent);
    }
}