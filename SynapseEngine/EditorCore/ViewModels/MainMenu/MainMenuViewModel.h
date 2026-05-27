#pragma once
#include "EditorCore/ViewModels/IViewModel.h"
#include "MainMenuState.h"
#include "MainMenuIntent.h"
#include "EditorCore/Api/ISceneAPI.h"

namespace Syn {
    class MainMenuViewModel : public IViewModel<MainMenuState, MainMenuIntent> {
    public:
        MainMenuViewModel(ISceneAPI* sceneApi) : _sceneApi(sceneApi) {}

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
                    _sceneApi->LoadScene();
                }
                else if constexpr (std::is_same_v<T, SaveSceneIntent>) {
                    _sceneApi->SaveScene();
                }
                }, intent);
        }

    private:
        ISceneAPI* _sceneApi = nullptr;
        MainMenuState _state;
    };
}