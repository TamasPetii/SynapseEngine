#pragma once
#include "EditorCore/ViewModels/IViewModel.h"
#include "SettingsState.h"
#include "SettingsIntent.h"
#include "EditorCore/API/ISettingsAPI.h"

namespace Syn {
    class SettingsViewModel : public IViewModel<SettingsState, SettingsIntent> {
    public:
        SettingsViewModel(ISettingsAPI* api) : _api(api) {}

        const SettingsState& GetState() const override { return _state; }

        void SyncWithEngine() override {
            if (_api) {
                _state.sceneSettings = _api->GetSceneSettings();
            }
        }

        void Dispatch(const SettingsIntent& intent) override {
            std::visit([this](auto&& arg) {
                using T = std::decay_t<decltype(arg)>;
                if constexpr (std::is_same_v<T, UpdateSceneSettingsIntent>)
                {
                    if (_api) {
                        _api->SetSceneSettings(arg.newSettings);
                    }
                }
                }, intent);
        }

    private:
        ISettingsAPI* _api = nullptr;
        SettingsState _state;
    };
}