#include "MaterialWorkspace.h"
#include "Editor/Manager/EditorIcons.h"

#include "Editor/View/ContentBrowser/ContentBrowserView.h"
#include "EditorCore/ViewModels/ContentBrowser/ContentBrowserViewModel.h"

#include "Editor/View/MaterialGraph/MaterialGraphView.h"
#include "EditorCore/ViewModels/MaterialGraph/MaterialGraphViewModel.h"

#include "Editor/View/MaterialHierarchy/MaterialHierarchyView.h"
#include "EditorCore/ViewModels/MaterialHierarchy/MaterialHierarchyViewModel.h"

#include "Editor/View/MaterialProperties/MaterialPropertiesView.h"
#include "EditorCore/ViewModels/MaterialProperties/MaterialPropertiesViewModel.h"

#include "Editor/View/MaterialViewport/MaterialViewportView.h"
#include "EditorCore/ViewModels/MaterialViewport/MaterialViewportViewModel.h"

namespace Syn {

    MaterialWorkspace::MaterialWorkspace(EditorContext* context, IconManager* iconManager, const std::string& assetPath)
        : _context(context), _iconManager(iconManager), _assetPath(assetPath) {}

    void MaterialWorkspace::OnActivate() {
        if (_context && _context->GetSceneApi()) {
            _context->GetSceneApi()->ActivateScene("MaterialPreview");
        }
    }

    void MaterialWorkspace::Initialize() {
        using ContentBrowserWin = EditorWindow<ContentBrowserView, ContentBrowserViewModel>;
        AddWindow<ContentBrowserWin>(
            ContentBrowserView{ _iconManager, SYN_ICON_FOLDER_OPEN " Content Browser###Content_Material" },
            ContentBrowserViewModel{ _context->GetFileSystemApi(), _assetPath }
        );

        using MaterialHierarchyWin = EditorWindow<MaterialHierarchyView, MaterialHierarchyViewModel>;
        AddWindow<MaterialHierarchyWin>(
            MaterialHierarchyView{},
            MaterialHierarchyViewModel{ _context->GetMaterialApi() }
        );

        using MaterialGraphWin = EditorWindow<MaterialGraphView, MaterialGraphViewModel>;
        AddWindow<MaterialGraphWin>(
            MaterialGraphView{},
            MaterialGraphViewModel{ _context->GetMaterialApi(), _context->GetTextureApi() }
        );

        using MaterialPropertiesWin = EditorWindow<MaterialPropertiesView, MaterialPropertiesViewModel>;
        AddWindow<MaterialPropertiesWin>(
            MaterialPropertiesView{},
            MaterialPropertiesViewModel{ _context->GetMaterialApi(), _context->GetTextureApi() }
        );

        using MaterialViewportWin = EditorWindow<MaterialViewportView, MaterialViewportViewModel>;
        AddWindow<MaterialViewportWin>(
            MaterialViewportView{},
            MaterialViewportViewModel{ _context->GetRenderApi() }
        );
    }

}