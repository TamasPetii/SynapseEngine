#pragma once
#include "../IView.h"
#include "EditorCore/ViewModels/MainMenu/MainMenuViewModel.h"
#include <imgui.h>

namespace Syn {
    class MainMenuView : public IView<MainMenuViewModel> {
    public:
        void Draw(MainMenuViewModel& vm) override {

            if (ImGui::BeginMainMenuBar()) {

                if (ImGui::BeginMenu("File")) {

                    if (ImGui::MenuItem("New Scene", "Ctrl+N")) {
                        vm.Dispatch(NewSceneIntent{});
                    }

                    if (ImGui::MenuItem("Load Scene...", "Ctrl+O")) {
                        vm.Dispatch(LoadSceneIntent{});
                    }

                    if (ImGui::MenuItem("Save Scene", "Ctrl+S")) {
                        vm.Dispatch(SaveSceneIntent{});
                    }

                    ImGui::EndMenu();
                }

                ImGui::EndMainMenuBar();
            }
        }
    };
}