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

#pragma once
#include "EditorCore/ViewModels/IViewModel.h"
#include "MainMenuState.h"
#include "MainMenuIntent.h"
#include "EditorCore/Api/ISceneApi.h"
#include "EditorCore/Api/IFileDialogApi.h"

namespace Syn {
    class MainMenuViewModel : public IViewModel<MainMenuState, MainMenuIntent> {
    public:
        MainMenuViewModel(ISceneApi* sceneApi, IFileDialogApi* fileDialogApi) 
            : _sceneApi(sceneApi), _fileDialogApi(fileDialogApi) {}

        const MainMenuState& GetState() const override { 
            return _state;
        }

        void SyncWithEngine() override {

        }

        void Dispatch(const MainMenuIntent& intent) override {
            std::visit([this](auto&& arg) {
                using T = std::decay_t<decltype(arg)>;
                if constexpr (std::is_same_v<T, NewSceneIntent>) {
                    _sceneApi->NewScene();
                }
                else if constexpr (std::is_same_v<T, LoadSceneIntent>) {
                    FileDialogArgs args{ "Load Scene", ".synscene", "." };

                    _fileDialogApi->OpenFile(args, [this](const std::string& path) {
                        _sceneApi->LoadScene(path);
                        });
                }
                else if constexpr (std::is_same_v<T, SaveSceneIntent>) {
                    FileDialogArgs args{ "Save Scene", ".synscene", "." };

                    _fileDialogApi->SaveFile(args, [this](const std::string& path) {
                        _sceneApi->SaveScene(path);
                        });
                }
                }, intent);
        }

    private:
        ISceneApi* _sceneApi = nullptr;
        IFileDialogApi* _fileDialogApi = nullptr;
        MainMenuState _state;
    };
}