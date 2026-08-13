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