#include "MaterialViewportViewModel.h"

namespace Syn {

    MaterialViewportViewModel::MaterialViewportViewModel(IRenderApi* renderApi)
        : _renderApi(renderApi) {}

    const MaterialViewportState& MaterialViewportViewModel::GetState() const {
        return _state;
    }

    void MaterialViewportViewModel::SyncWithEngine() {
        if (!_renderApi) return;

        _state.textureId = _renderApi->GetViewportTexture(
            _state.currentGroup,
            _state.currentTarget,
            _state.currentView
        );
    }

    void MaterialViewportViewModel::Dispatch(const MaterialViewportIntent& intent) {
        std::visit([this](auto&& arg) {
            using T = std::decay_t<decltype(arg)>;
            if constexpr (std::is_same_v<T, ResizeMaterialViewportIntent>) {
                HandleResize(arg);
            }
            }, intent);
    }

    void MaterialViewportViewModel::HandleResize(const ResizeMaterialViewportIntent& intent) {
        if (intent.width > 0 && intent.height > 0 &&
            (_state.width != intent.width || _state.height != intent.height))
        {
            _state.width = intent.width;
            _state.height = intent.height;

            _renderApi->ResizeRenderTargets(_state.width, _state.height);
        }
    }

}