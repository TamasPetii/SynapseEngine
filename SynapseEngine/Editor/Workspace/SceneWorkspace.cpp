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

namespace Syn {

    SceneWorkspace::SceneWorkspace(EditorContext* context, IconManager* iconManager, const std::string& assetPath)
        : _context(context), _iconManager(iconManager), _assetPath(assetPath) {}

    void SceneWorkspace::Initialize() 
    {
        using ContentBrowserWin = EditorWindow<ContentBrowserView, ContentBrowserViewModel>;
        AddWindow<ContentBrowserWin>(
            ContentBrowserView{ _iconManager, SYN_ICON_FOLDER_OPEN " Content Browser###Content_Scene" },
            ContentBrowserViewModel{ _context->GetFileSystemApi(), _assetPath }
        );

        using ComponentWin = EditorWindow<ComponentView, ComponentViewModel>;
        AddWindow<ComponentWin>(
            ComponentView{},
            ComponentViewModel{
                _context->GetSelectionApi(), _context->GetTagApi(), _context->GetTransformApi(),
                _context->GetHierarchyApi(), _context->GetDirectionLightApi(),
                _context->GetPointLightApi(), _context->GetSpotLightApi()
            }
        );

        using ViewportWin = EditorWindow<ViewportView, ViewportViewModel>;
        AddWindow<ViewportWin>(
            ViewportView{},
            ViewportViewModel{
                _context->GetRenderApi(), _context->GetSelectionApi(), _context->GetTransformApi(),
                _context->GetSettingsApi(), _context->GetHierarchyApi()
            }
        );

        using SettingsWin = EditorWindow<SettingsView, SettingsViewModel>;
        AddWindow<SettingsWin>(
            SettingsView{},
            SettingsViewModel{ _context->GetSettingsApi() }
        );

        using HierarchyWin = EditorWindow<HierarchyView, HierarchyViewModel>;
        AddWindow<HierarchyWin>(
            HierarchyView{},
            HierarchyViewModel{ _context->GetHierarchyApi(), _context->GetSelectionApi(), _context->GetTagApi() }
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
                _context->GetLoggerApi()
            }
        );
    }

}