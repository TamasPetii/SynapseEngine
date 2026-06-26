#pragma once
#include <imgui.h>

namespace Syn {
    enum class EditorIconType {
        Folder,
        File,
        Image,
        Code,
        Model,
        Sound
    };

    class IIconManager {
    public:
        virtual ~IIconManager() = default;

        virtual ImTextureID GetIconDescriptor(EditorIconType type) const = 0;
        virtual ImFont* GetMainIconFont() const = 0;
    };
}