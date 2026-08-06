#include "AnimationSequencerViewModel.h"

namespace Syn {

    AnimationSequencerViewModel::AnimationSequencerViewModel(IAnimationApi* animationApi)
        : _animationApi(animationApi) {}

    const AnimationSequencerState& AnimationSequencerViewModel::GetState() const {
        return _state;
    }

    void AnimationSequencerViewModel::SyncWithEngine() {
        if (!_animationApi) return;

        _state.activeAnimationId = _animationApi->GetSelected();

        if (_state.activeAnimationId != 0xFFFFFFFF) {
            _state.currentAnimData = _animationApi->GetAnimationCpuData(_state.activeAnimationId);
        }
        else {
            _state.currentAnimData = nullptr;
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
            }, intent);
    }
}