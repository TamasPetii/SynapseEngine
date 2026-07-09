#include "ModelWorkspace.h"
#include "Editor/Manager/EditorIcons.h"

#include "Editor/View/ContentBrowser/ContentBrowserView.h"
#include "EditorCore/ViewModels/ContentBrowser/ContentBrowserViewModel.h"

#include "Editor/View/ModelHierarchy/ModelHierarchyView.h"
#include "EditorCore/ViewModels/ModelHierarchy/ModelHierarchyViewModel.h"

#include "Editor/View/ModelProperties/ModelPropertiesView.h"
#include "EditorCore/ViewModels/ModelProperties/ModelPropertiesViewModel.h"

#include "Editor/View/ModelViewport/ModelViewportView.h"
#include "EditorCore/ViewModels/ModelViewport/ModelViewportViewModel.h"

#include "Engine/Scene/SceneNames.h"

namespace Syn {

    ModelWorkspace::ModelWorkspace(EditorContext* context, IconManager* iconManager, const std::string& assetPath)
        : _context(context), _iconManager(iconManager), _assetPath(assetPath) {}

    void ModelWorkspace::OnActivate() {
        if (_context && _context->GetApi<ISceneApi>()) {
            _context->GetApi<ISceneApi>()->ActivateScene(SceneNames::ModelPreview);
        }
    }

    void ModelWorkspace::Initialize() {

        using ContentBrowserWin = EditorWindow<ContentBrowserView, ContentBrowserViewModel>;
        AddWindow<ContentBrowserWin>(
            ContentBrowserView{ _iconManager, SYN_ICON_FOLDER_OPEN " Content Browser###Content_Model" },
            ContentBrowserViewModel{ _context->GetApi<IFileSystemApi>(), _assetPath }
        );

        using ModelHierarchyWin = EditorWindow<ModelHierarchyView, ModelHierarchyViewModel>;
        AddWindow<ModelHierarchyWin>(
            ModelHierarchyView{},
            ModelHierarchyViewModel{ _context->GetApi<IModelApi>() }
        );

        using ModelPropertiesWin = EditorWindow<ModelPropertiesView, ModelPropertiesViewModel>;
        AddWindow<ModelPropertiesWin>(
            ModelPropertiesView{},
            ModelPropertiesViewModel{ _context->GetApi<IModelApi>() }
        );

        using ModelViewportWin = EditorWindow<ModelViewportView, ModelViewportViewModel>;
        AddWindow<ModelViewportWin>(
            ModelViewportView{},
            ModelViewportViewModel{
                _context->GetApi<IRenderApi>(),
                _context->GetApi<ISelectionApi>(),
                _context->GetApi<ITransformApi>(),
                _context->GetApi<ISettingsApi>(),
                _context->GetApi<IModelApi>()
            }
        );
    }
}