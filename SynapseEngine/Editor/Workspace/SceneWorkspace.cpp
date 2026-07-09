#include "SceneWorkspace.h"
#include "Editor/Manager/EditorIcons.h"

#include "Editor/View/ContentBrowser/ContentBrowserView.h"
#include "EditorCore/ViewModels/ContentBrowser/ContentBrowserViewModel.h"
#include "Editor/View/Component/ComponentView.h"
#include "EditorCore/ViewModels/Component/ComponentViewModel.h"
#include "Editor/View/Viewport/ViewportView.h"
#include "EditorCore/ViewModels/Viewport/ViewportViewModel.h"
#include "Editor/View/Settings/SettingsView.h"
#include "EditorCore/ViewModels/Settings/SettingsViewModel.h"
#include "Editor/View/Hierarchy/HierarchyView.h"
#include "EditorCore/ViewModels/Hierarchy/HierarchyViewModel.h"
#include "Editor/View/Benchmark/BenchmarkView.h"
#include "EditorCore/ViewModels/Benchmark/BenchmarkViewModel.h"
#include "Editor/View/Logger/LoggerView.h"
#include "EditorCore/ViewModels/Logger/LoggerViewModel.h"

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