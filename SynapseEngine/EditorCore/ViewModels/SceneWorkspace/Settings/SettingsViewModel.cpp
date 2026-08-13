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

#include "SettingsViewModel.h"

namespace Syn {

    SettingsViewModel::SettingsViewModel(ISettingsApi* api)
        : _api(api)
    {}

    const SettingsState& SettingsViewModel::GetState() const {
        return _state;
    }

    void SettingsViewModel::SyncWithEngine() {
        if (_api) {
            _state.sceneSettings = _api->GetSceneSettings();
			_state.availableSkyTextures = _api->GetAvailableSkyTextures();
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