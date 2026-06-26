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