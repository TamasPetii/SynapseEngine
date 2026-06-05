#pragma once
#include "EditorCore/ViewModels/IViewModel.h"
#include "SettingsState.h"
#include "SettingsIntent.h"
#include "EditorCore/Api/ISettingsApi.h"

namespace Syn {
    class SettingsViewModel : public IViewModel<SettingsState, SettingsIntent> {
    public:
        SettingsViewModel(ISettingsApi* api);
        ~SettingsViewModel() override = default;

        const SettingsState& GetState() const override;

        void SyncWithEngine() override;
        void Dispatch(const SettingsIntent& intent) override;

    private:
        ISettingsApi* _api = nullptr;
        SettingsState _state;
    };
}