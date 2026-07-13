#include "SceneWorkspace.h"
#include "Editor/Manager/EditorIcons.h"

#include "Editor/Workspace/Common/ContentBrowser/ContentBrowserView.h"
#include "EditorCore/ViewModels/Common/ContentBrowser/ContentBrowserViewModel.h"

#include "View/Component/ComponentView.h"
#include "EditorCore/ViewModels/SceneWorkspace/Component/ComponentViewModel.h"

#include "View/Viewport/ViewportView.h"
#include "EditorCore/ViewModels/SceneWorkspace/Viewport/ViewportViewModel.h"

#include "View/Settings/SettingsView.h"
#include "EditorCore/ViewModels/SceneWorkspace/Settings/SettingsViewModel.h"

#include "View/Hierarchy/HierarchyView.h"
#include "EditorCore/ViewModels/SceneWorkspace/Hierarchy/HierarchyViewModel.h"

#include "View/Benchmark/BenchmarkView.h"
#include "EditorCore/ViewModels/SceneWorkspace/Benchmark/BenchmarkViewModel.h"

#include "View/Logger/LoggerView.h"
#include "EditorCore/ViewModels/SceneWorkspace/Logger/LoggerViewModel.h"

#include "Engine/Scene/SceneNames.h"

namespace Syn {

    SceneWorkspace::SceneWorkspace(EditorContext* context, IconManager* iconManager, const std::string& assetPath)
        : _context(context), _iconManager(iconManager), _assetPath(assetPath) {}

    void SceneWorkspace::OnActivate() {
        if (_context && _context->GetApi<ISceneApi>()) {
            _context->GetApi<ISceneApi>()->ActivateScene(SceneNames::Main);
        }
    }

    void SceneWorkspace::Initialize()
    {
        using ContentBrowserWin = EditorWindow<ContentBrowserView, ContentBrowserViewModel>;
        AddWindow<ContentBrowserWin>(
            ContentBrowserView{ _iconManager, SYN_ICON_FOLDER_OPEN " Content Browser###Content_Scene" },
            ContentBrowserViewModel{ _context->GetApi<IFileSystemApi>(), _assetPath }
        );

        using ComponentWin = EditorWindow<ComponentView, ComponentViewModel>;
        AddWindow<ComponentWin>(
            ComponentView{},
            ComponentViewModel{
                _context->GetApi<ISelectionApi>(),
                _context->GetApi<ITagApi>(),
                _context->GetApi<ITransformApi>(),
                _context->GetApi<IHierarchyApi>(),
                _context->GetApi<IDirectionLightApi>(),
                _context->GetApi<IPointLightApi>(),
                _context->GetApi<ISpotLightApi>(),
                _context->GetApi<ICameraApi>(),
                _context->GetApi<IBoxColliderApi>(),
                _context->GetApi<ISphereColliderApi>(),
                _context->GetApi<ICapsuleColliderApi>(),
                _context->GetApi<IConvexColliderApi>(),
                _context->GetApi<IMeshColliderApi>(),
                _context->GetApi<IRigidBodyApi>(),
                _context->GetApi<IModelComponentApi>(),
                _context->GetApi<IAnimationApi>(),
                _context->GetApi<IMaterialOverrideApi>()
            }
        );

        using ViewportWin = EditorWindow<ViewportView, ViewportViewModel>;
        AddWindow<ViewportWin>(
            ViewportView{},
            ViewportViewModel{
                _context->GetApi<IRenderApi>(),
                _context->GetApi<ISelectionApi>(),
                _context->GetApi<ITransformApi>(),
                _context->GetApi<ISettingsApi>(),
                _context->GetApi<IHierarchyApi>()
            }
        );

        using SettingsWin = EditorWindow<SettingsView, SettingsViewModel>;
        AddWindow<SettingsWin>(
            SettingsView{},
            SettingsViewModel{ _context->GetApi<ISettingsApi>() }
        );

        using HierarchyWin = EditorWindow<HierarchyView, HierarchyViewModel>;
        AddWindow<HierarchyWin>(
            HierarchyView{},
            HierarchyViewModel{
                _context->GetApi<IHierarchyApi>(),
                _context->GetApi<ISelectionApi>(),
                _context->GetApi<ITagApi>()
            }
        );

        using BenchmarkWin = EditorWindow<BenchmarkView, BenchmarkViewModel>;
        AddWindow<BenchmarkWin>(
            BenchmarkView{},
            BenchmarkViewModel{}
        );

        using LoggerWin = EditorWindow<LoggerView, LoggerViewModel>;
        AddWindow<LoggerWin>(
            LoggerView{},
            LoggerViewModel{
                _context->GetApi<ILoggerApi>()
            }
        );
    }

}