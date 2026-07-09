#pragma once
#include <functional>

namespace Syn::UI 
{
    void ItemCardContainer(const char* strId,
        int itemCount,
        float thumbnailSize,
        const std::function<void(int index)>& drawItem,
        float spacing = 12.0f);
}