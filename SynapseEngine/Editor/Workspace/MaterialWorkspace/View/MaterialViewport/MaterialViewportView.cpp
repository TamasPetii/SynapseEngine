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