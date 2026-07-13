#include "MainMenuView.h"

namespace Syn
{
    void MainMenuView::Draw(MainMenuViewModel& vm) {

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
}