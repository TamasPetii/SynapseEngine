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

#include "ItemCardContainerWidget.h"
#include "ItemCardWidget.h"
#include <imgui.h>
#include <algorithm>

namespace Syn::UI {

    void ItemCardContainer(const char* strId,
        int itemCount,
        float thumbnailSize,
        const std::function<void(int index)>& drawItem,
        float spacing) 
    {
        if (itemCount <= 0 || !drawItem) return;

        ImGui::PushID(strId);

        const float cardW = ItemCardWidth(thumbnailSize);
        const float cardH = ItemCardHeight(thumbnailSize);
        const float avail = ImGui::GetContentRegionAvail().x;

        int columns = static_cast<int>((avail + spacing) / (cardW + spacing));
        columns = std::max(1, columns);

        const int rows = (itemCount + columns - 1) / columns;

        ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(spacing, spacing));

        ImGuiListClipper clipper;
        clipper.Begin(rows, cardH + spacing);
        while (clipper.Step()) {
            for (int row = clipper.DisplayStart; row < clipper.DisplayEnd; ++row) {
                for (int col = 0; col < columns; ++col) {
                    const int index = row * columns + col;
                    if (index >= itemCount) break;
                    if (col > 0) ImGui::SameLine();
                    drawItem(index);
                }
            }
        }
        clipper.End();

        ImGui::PopStyleVar();
        ImGui::PopID();
    }
}