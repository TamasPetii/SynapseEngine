#include "ModelWorkspace.h"
#include "Editor/Manager/EditorIcons.h"

#include "Editor/View/ContentBrowser/ContentBrowserView.h"
#include "EditorCore/ViewModels/ContentBrowser/ContentBrowserViewModel.h"

#include "Editor/View/ModelHierarchy/ModelHierarchyView.h"
#include "EditorCore/ViewModels/ModelHierarchy/ModelHierarchyViewModel.h"

#include "Editor/View/ModelProperties/ModelPropertiesView.h"
#include "EditorCore/ViewModels/ModelProperties/ModelPropertiesViewModel.h"


namespace Syn {

    ModelWorkspace::ModelWorkspace(EditorContext* context, IconManager* iconManager, const std::string& assetPath)
        : _context(context), _iconManager(iconManager), _assetPath(assetPath) {}

    void ModelWorkspace::Initialize() {

        using ContentBrowserWin = EditorWindow<ContentBrowserView, ContentBrowserViewModel>;
        AddWindow<ContentBrowserWin>(
            ContentBrowserView{ _iconManager, SYN_ICON_FOLDER_OPEN " Content Browser###Content_Model" },
            ContentBrowserViewModel{ _context->GetFileSystemApi(), _assetPath }
        );

        using ModelHierarchyWin = EditorWindow<ModelHierarchyView, ModelHierarchyViewModel>;
        AddWindow<ModelHierarchyWin>(
            ModelHierarchyView{},
            ModelHierarchyViewModel{ _context->GetModelApi() }
        );

        using ModelPropertiesWin = EditorWindow<ModelPropertiesView, ModelPropertiesViewModel>;
        AddWindow<ModelPropertiesWin>(
            ModelPropertiesView{},
            ModelPropertiesViewModel{ _context->GetModelApi() }
        );
    }

}