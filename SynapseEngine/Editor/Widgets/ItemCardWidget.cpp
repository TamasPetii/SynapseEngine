#include "ItemCardWidget.h"
#include <cstring>
#include <algorithm>

namespace Syn::UI {

    namespace {
        constexpr float kPadding = 6.0f;
        constexpr float kRounding = 6.0f;
        constexpr float kImageRounding = 4.0f;
        constexpr float kTextGap = 4.0f;
        constexpr int   kMaxTitleLines = 2;

        constexpr ImU32 kColBg = IM_COL32(36, 36, 40, 255);
        constexpr ImU32 kColBgHovered = IM_COL32(50, 50, 56, 255);
        constexpr ImU32 kColBgActive = IM_COL32(58, 62, 70, 255);
        constexpr ImU32 kColBgSelected = IM_COL32(38, 56, 80, 255);
        constexpr ImU32 kColAccent = IM_COL32(66, 150, 250, 255);
        constexpr ImU32 kColBorder = IM_COL32(255, 255, 255, 18);
        constexpr ImU32 kColText = IM_COL32(230, 230, 230, 255);
        constexpr ImU32 kColPlaceholder = IM_COL32(20, 20, 22, 255);

        const char* Utf8Next(const char* s, const char* end) {
            if (s >= end) return end;
            unsigned char c = static_cast<unsigned char>(*s);
            int len = 1;
            if (c >= 0xF0)      len = 4;
            else if (c >= 0xE0) len = 3;
            else if (c >= 0xC0) len = 2;
            const char* n = s + len;
            return n > end ? end : n;
        }

        const char* ComputeLineEnd(const char* lineStart, const char* textEnd,
            float maxWidth, const char** outNextStart) {
            const char* s = lineStart;
            const char* lastSpace = nullptr;
            while (s < textEnd) {
                const char* next = Utf8Next(s, textEnd);
                if (ImGui::CalcTextSize(lineStart, next).x > maxWidth && s > lineStart) {
                    if (lastSpace && lastSpace > lineStart) {
                        *outNextStart = lastSpace + 1;
                        return lastSpace;
                    }
                    *outNextStart = s;
                    return s;
                }
                if (*s == ' ') lastSpace = s;
                s = next;
            }
            *outNextStart = textEnd;
            return textEnd;
        }
    }

    float ItemCardWidth(float thumbnailSize) {
        return thumbnailSize + kPadding * 2.0f;
    }

    float ItemCardHeight(float thumbnailSize) {
        return kPadding + thumbnailSize + kTextGap + ImGui::GetTextLineHeight() * kMaxTitleLines + kPadding;
    }

    bool ItemCard(const ItemCardDesc& desc, float thumbnailSize) {
        const char* idStr = desc.id ? desc.id : desc.title;
        ImGui::PushID(idStr);

        const ImVec2 size(ItemCardWidth(thumbnailSize), ItemCardHeight(thumbnailSize));
        const ImVec2 min = ImGui::GetCursorScreenPos();
        const ImVec2 max(min.x + size.x, min.y + size.y);

        bool clicked = ImGui::InvisibleButton("##ItemCard", size);
        const bool hovered = ImGui::IsItemHovered();
        const bool active = ImGui::IsItemActive();
        const bool doubleClicked = hovered && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left);

        if (desc.events.onDragDropSource) {
            if (ImGui::BeginDragDropSource()) {
                desc.events.onDragDropSource();
                ImGui::EndDragDropSource();
            }
        }

        ImDrawList* dl = ImGui::GetWindowDrawList();

        {
            const float yOffset = 2.0f;
            const int   layers = 3;
            const int   alpha = hovered ? 20 : 13;
            for (int i = 0; i < layers; ++i) {
                const float expand = 1.5f * static_cast<float>(i + 1);
                dl->AddRectFilled(
                    ImVec2(min.x - expand, min.y - expand + yOffset),
                    ImVec2(max.x + expand, max.y + expand + yOffset),
                    IM_COL32(0, 0, 0, alpha), kRounding + expand);
            }
        }

        // Background
        ImU32 bgCol = kColBg;
        if (desc.selected)  bgCol = kColBgSelected;
        if (hovered)        bgCol = desc.selected ? kColBgSelected : kColBgHovered;
        if (active)         bgCol = kColBgActive;
        dl->AddRectFilled(min, max, bgCol, kRounding);

        // Border
        if (desc.selected) {
            dl->AddRect(min, max, kColAccent, kRounding, 0, 2.0f);
        }
        else {
            dl->AddRect(min, max, kColBorder, kRounding, 0, 1.0f);
        }

        // Thumbnail
        const ImVec2 imgMin(min.x + kPadding, min.y + kPadding);
        const ImVec2 imgMax(imgMin.x + thumbnailSize, imgMin.y + thumbnailSize);
        if (desc.texture) {
            dl->AddImageRounded(desc.texture, imgMin, imgMax,
                desc.uv0, desc.uv1,
                ImGui::ColorConvertFloat4ToU32(desc.imageTint),
                kImageRounding);
        }
        else {
            dl->AddRectFilled(imgMin, imgMax, kColPlaceholder, kImageRounding);
        }

        const char* text = desc.title;
        const char* textEnd = text + std::strlen(text);
        const float maxTextWidth = thumbnailSize;
        const float lineHeight = ImGui::GetTextLineHeight();
        float textY = imgMax.y + kTextGap;

        bool truncated = false;
        const char* lineStart = text;

        for (int line = 0; line < kMaxTitleLines && lineStart < textEnd; ++line) {
            const bool lastLine = (line == kMaxTitleLines - 1);

            if (!lastLine) {
                const char* nextStart = textEnd;
                const char* lineEnd = ComputeLineEnd(lineStart, textEnd,
                    maxTextWidth, &nextStart);
                const float w = ImGui::CalcTextSize(lineStart, lineEnd).x;
                const float x = min.x + kPadding
                    + std::max(0.0f, (maxTextWidth - w) * 0.5f);
                dl->AddText(ImVec2(x, textY), kColText, lineStart, lineEnd);
                lineStart = nextStart;
            }
            else {
                float w = ImGui::CalcTextSize(lineStart, textEnd).x;
                const char* lineEnd = textEnd;

                if (w > maxTextWidth) {
                    truncated = true;
                    const float ellipsisW = ImGui::CalcTextSize("...").x;
                    const char* s = lineStart;
                    lineEnd = lineStart;
                    while (s < textEnd) {
                        const char* next = Utf8Next(s, textEnd);
                        if (ImGui::CalcTextSize(lineStart, next).x + ellipsisW
                    > maxTextWidth) break;
                    lineEnd = next;
                    s = next;
                    }
                    w = ImGui::CalcTextSize(lineStart, lineEnd).x + ellipsisW;
                }

                const float x = min.x + kPadding
                    + std::max(0.0f, (maxTextWidth - w) * 0.5f);
                dl->AddText(ImVec2(x, textY), kColText, lineStart, lineEnd);
                if (truncated) {
                    const float usedW = ImGui::CalcTextSize(lineStart, lineEnd).x;
                    dl->AddText(ImVec2(x + usedW, textY), kColText, "...");
                }
                lineStart = textEnd;
            }
            textY += lineHeight;
        }
        if (lineStart < textEnd) truncated = true;

        if (truncated && hovered && !active) {
            ImGui::SetTooltip("%s", desc.title);
        }

        if (doubleClicked && desc.events.onDoubleClick) desc.events.onDoubleClick();
        else if (clicked && desc.events.onClick)        desc.events.onClick();

        ImGui::PopID();
        return clicked;
    }
}