#pragma once
#include <imgui.h>
#include <functional>

namespace Syn::UI {
    struct ItemCardEvents {
        std::function<void()> onClick;
        std::function<void()> onDoubleClick;
        std::function<void()> onDragDropSource;
    };

    struct ItemCardDesc {
        const char* id = nullptr;
        const char* title = "";

        ImTextureID texture = 0;
        ImVec2 uv0 = ImVec2(0.0f, 0.0f);
        ImVec2 uv1 = ImVec2(1.0f, 1.0f);
        ImVec4 imageTint = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);

        bool selected = false;
        ItemCardEvents events;
    };

    bool ItemCard(const ItemCardDesc& desc, float thumbnailSize);
    float ItemCardWidth(float thumbnailSize);
    float ItemCardHeight(float thumbnailSize);
}