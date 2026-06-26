#pragma once
#include "IWorkspace.h"
#include "Editor/EditorApi/EditorContext.h"
#include "Editor/Manager/IconManager.h"
#include <string>

namespace Syn {

    class TextureWorkspace : public IWorkspace {
    public:
        TextureWorkspace(EditorContext* context, IconManager* iconManager, const std::string& assetPath);
        ~TextureWorkspace() override = default;

        void Initialize() override;

    private:
        EditorContext* _context;
        IconManager* _iconManager;
        std::string _assetPath;
    };

}