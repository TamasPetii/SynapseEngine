#include "ComponentViewModel.h"

namespace Syn 
{
    ComponentViewModel::ComponentViewModel(ISelectionAPI* selectionApi, ITagAPI* tagApi, ITransformAPI* transformApi)
        : _selectionApi(selectionApi),
        _tagVM(selectionApi, tagApi),
        _transformVM(selectionApi, transformApi)
    {}

    const ComponentState& ComponentViewModel::GetState() const {
        return _state;
    }

    void ComponentViewModel::SyncWithEngine() {
        if (!_selectionApi) return;

        EntityID activeEntity = _selectionApi->GetSelectedEntity();

        _state.activeEntityId = activeEntity;
        _state.hasSelection = (activeEntity != NULL_ENTITY);

        if (_state.hasSelection) {
            _tagVM.SyncWithEngine();
            _transformVM.SyncWithEngine();
        }
    }

    void ComponentViewModel::Dispatch(const ComponentIntent& intent) {
        std::visit([this](auto&& arg) {
            using T = std::decay_t<decltype(arg)>;

            if constexpr (std::is_same_v<T, TagIntent>) {
                _tagVM.Dispatch(arg);
            }
            else if constexpr (std::is_same_v<T, TransformIntent>) {
                _transformVM.Dispatch(arg);
            }
            }, intent);
    }

    TagViewModel& ComponentViewModel::GetTagVM() {
        return _tagVM;
    }

    TransformViewModel& ComponentViewModel::GetTransformVM() {
        return _transformVM;
    }

}