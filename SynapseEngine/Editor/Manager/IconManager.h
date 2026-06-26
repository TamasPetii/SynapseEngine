#pragma once
#include "IIconManager.h"
#include "GuiTextureManager.h"
#include "Engine/Image/ImageManager.h"
#include <unordered_map>
#include <string>

namespace Syn {
    class IconManager : public IIconManager {
    public:
        IconManager(ImageManager* imageManager, GuiTextureManager* guiTextureManager);
        ~IconManager() override = default;

        void InitializeFontAwesome(ImGuiIO& io, const std::string& fontPath, float fontSize);
        void LoadEngineIcons(const std::string& iconDirectory);

        ImTextureID GetIconDescriptor(EditorIconType type) const override;
        ImFont* GetMainIconFont() const override { return _fontAwesome; }

    private:
        ImageManager* _imageManager = nullptr;
        GuiTextureManager* _guiTextureManager = nullptr;
        ImFont* _fontAwesome = nullptr;

        std::unordered_map<EditorIconType, ImTextureID> _iconCache;
    };
}