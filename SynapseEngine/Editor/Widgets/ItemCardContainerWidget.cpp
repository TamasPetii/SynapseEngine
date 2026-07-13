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