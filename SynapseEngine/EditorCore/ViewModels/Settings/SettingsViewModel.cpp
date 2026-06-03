#include "SettingsViewModel.h"

namespace Syn {

    SettingsViewModel::SettingsViewModel(ISettingsAPI* api)
        : _api(api)
    {}

    const SettingsState& SettingsViewModel::GetState() const {
        return _state;
    }

    void SettingsViewModel::SyncWithEngine() {
        if (_api) {
            _state.sceneSettings = _api->GetSceneSettings();
        }
    }

    void SettingsViewModel::Dispatch(const SettingsIntent& intent) {
        std::visit([this](auto&& arg) {
            using T = std::decay_t<decltype(arg)>;

            if constexpr (std::is_same_v<T, UpdateSceneSettingsIntent>) {
                if (_api) {
                    _api->SetSceneSettings(arg.newSettings);
                }
            }
            }, intent);
    }
}