#pragma once
#include "Editor/Workspace/IWorkspace.h"
#include "Editor/EditorApi/EditorContext.h"
#include "Editor/Manager/IconManager.h"
#include <string>

namespace Syn {

    class ModelWorkspace : public IWorkspace {
    public:
        ModelWorkspace(EditorContext* context, IconManager* iconManager, const std::string& assetPath);
        ~ModelWorkspace() override = default;

        void Initialize() override;
        void OnActivate() override;
    private:
        EditorContext* _context;
        IconManager* _iconManager;
        std::string _assetPath;
    };

}