#pragma once
#include "EditorCore/ViewModels/IViewModel.h"
#include "MainMenuState.h"
#include "MainMenuIntent.h"
#include "EditorCore/Api/ISceneAPI.h"
#include "EditorCore/Api/IFileDialogAPI.h"

namespace Syn {
    class MainMenuViewModel : public IViewModel<MainMenuState, MainMenuIntent> {
    public:
        MainMenuViewModel(ISceneAPI* sceneApi, IFileDialogAPI* fileDialogApi) 
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
        ISceneAPI* _sceneApi = nullptr;
        IFileDialogAPI* _fileDialogApi = nullptr;
        MainMenuState _state;
    };
}