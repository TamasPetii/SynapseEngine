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

#include "MaterialViewportView.h"
#include "Editor/Manager/EditorIcons.h"
#include <imgui.h>

namespace Syn {

    void MaterialViewportView::Draw(MaterialViewportViewModel& vm) {
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2{ 0, 0 });

        ImGui::Begin(SYN_ICON_LAYER_GROUP " Material Viewport", nullptr);

        MaterialViewportState state = vm.GetState();

        ImVec2 viewportPanelSize = ImGui::GetContentRegionAvail();
        uint32_t currentWidth = static_cast<uint32_t>(viewportPanelSize.x);
        uint32_t currentHeight = static_cast<uint32_t>(viewportPanelSize.y);

        bool isResizing = (currentWidth > 0 && currentHeight > 0 &&
            (currentWidth != state.width || currentHeight != state.height));

        vm.Dispatch(ResizeMaterialViewportIntent{ currentWidth, currentHeight });

        if (viewportPanelSize.x <= 0.0f) viewportPanelSize.x = 1.0f;
        if (viewportPanelSize.y <= 0.0f) viewportPanelSize.y = 1.0f;

        if (state.textureId && !isResizing) {
            ImGui::GetWindowDrawList()->AddCallback(ImGui::GetPlatformIO().DrawCallback_SetSamplerNearest, nullptr);
            ImGui::Image(state.textureId, viewportPanelSize);
            ImGui::GetWindowDrawList()->AddCallback(ImGui::GetPlatformIO().DrawCallback_SetSamplerLinear, nullptr);
        }
        else {
            ImGui::Dummy(viewportPanelSize);
        }

        state.isHovered = ImGui::IsWindowHovered();
        state.isFocused = ImGui::IsWindowFocused();

        ImGui::End();
        ImGui::PopStyleVar();
    }

}