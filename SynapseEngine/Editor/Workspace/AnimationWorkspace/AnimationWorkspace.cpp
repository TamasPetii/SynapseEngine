#include "AnimationWorkspace.h"
#include "Editor/Manager/EditorIcons.h"

#include "Editor/Workspace/Common/ContentBrowser/ContentBrowserView.h"
#include "EditorCore/ViewModels/Common/ContentBrowser/ContentBrowserViewModel.h"

#include "View/AnimationHierarchy/AnimationHierarchyView.h"
#include "EditorCore/ViewModels/AnimationWorkspace/AnimationHierarchy/AnimationHierarchyViewModel.h"

#include "View/AnimationViewport/AnimationViewportView.h"
#include "EditorCore/ViewModels/AnimationWorkspace/AnimationViewport/AnimationViewportViewModel.h"

#include "Engine/Scene/SceneNames.h"

namespace Syn {

    AnimationWorkspace::AnimationWorkspace(EditorContext* context, IconManager* iconManager, const std::string& assetPath)
        : _context(context), _iconManager(iconManager), _assetPath(assetPath) {}

    void AnimationWorkspace::OnActivate() {
        if (_context && _context->GetApi<ISceneApi>()) {
            _context->GetApi<ISceneApi>()->ActivateScene(SceneNames::AnimationPreview);
        }
    }

    void AnimationWorkspace::Initialize() {

        using ContentBrowserWin = EditorWindow<ContentBrowserView, ContentBrowserViewModel>;
        AddWindow<ContentBrowserWin>(
            ContentBrowserView{ _iconManager, SYN_ICON_FOLDER_OPEN " Content Browser###Content_Animation" },
            ContentBrowserViewModel{ _context->GetApi<IFileSystemApi>(), _assetPath }
        );

        using AnimationHierarchyWin = EditorWindow<AnimationHierarchyView, AnimationHierarchyViewModel>;
        AddWindow<AnimationHierarchyWin>(
            AnimationHierarchyView{},
            AnimationHierarchyViewModel{ 
                _context->GetApi<IAnimationApi>(),
                _context->GetApi<IModelApi>(),
                _context->GetApi<IPreviewApi>()
            }
        );

        using AnimationViewportWin = EditorWindow<AnimationViewportView, AnimationViewportViewModel>;
        AddWindow<AnimationViewportWin>(
            AnimationViewportView{},
            AnimationViewportViewModel{
                _context->GetApi<IRenderApi>(),
                _context->GetApi<ISelectionApi>(),
                _context->GetApi<ITransformApi>(),
                _context->GetApi<ISettingsApi>(),
                _context->GetApi<IAnimationApi>()
            }
        );
    }
}