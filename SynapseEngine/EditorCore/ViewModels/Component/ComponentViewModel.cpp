#include "ComponentViewModel.h"

namespace Syn 
{
    ComponentViewModel::ComponentViewModel(ISelectionApi* selectionApi, ITagApi* tagApi, ITransformApi* transformApi, IDirectionLightApi* directionLightApi, IHierarchyApi* hierarchyApi)
        : _selectionApi(selectionApi),
        _tagViewModel(selectionApi, tagApi),
        _transformViewModel(selectionApi, transformApi, hierarchyApi),
        _directionLightViewModel(selectionApi, directionLightApi)
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
            _tagViewModel.SyncWithEngine();
            _transformViewModel.SyncWithEngine();
            _directionLightViewModel.SyncWithEngine();
        }
    }

    void ComponentViewModel::Dispatch(const ComponentIntent& intent) {
        std::visit([this](auto&& arg) {
            using T = std::decay_t<decltype(arg)>;

            if constexpr (std::is_same_v<T, TagIntent>) {
                _tagViewModel.Dispatch(arg);
            }
            else if constexpr (std::is_same_v<T, TransformIntent>) {
                _transformViewModel.Dispatch(arg);
            }
            else if constexpr (std::is_same_v<T, DirectionLightIntent>) {
                _directionLightViewModel.Dispatch(arg);
            }
            }, intent);
    }
}