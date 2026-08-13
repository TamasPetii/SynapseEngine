// Copyright (C) 2026 Tamás Péter
// This file is part of SynapseEngine.
//
// SynapseEngine is free software: you can redistribute it and/or modify
// it under the terms of the GNU Affero General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// SynapseEngine is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License
// along with SynapseEngine. If not, see <https://www.gnu.org/licenses/>.

#include "ModelWorkspace.h"
#include "Editor/Manager/EditorIcons.h"

#include "Editor/Workspace/Common/ContentBrowser/ContentBrowserView.h"
#include "EditorCore/ViewModels/Common/ContentBrowser/ContentBrowserViewModel.h"

#include "View/ModelHierarchy/ModelHierarchyView.h"
#include "EditorCore/ViewModels/ModelWorkspace/ModelHierarchy/ModelHierarchyViewModel.h"

#include "View/ModelProperties/ModelPropertiesView.h"
#include "EditorCore/ViewModels/ModelWorkspace/ModelProperties/ModelPropertiesViewModel.h"

#include "View/ModelViewport/ModelViewportView.h"
#include "EditorCore/ViewModels/ModelWorkspace/ModelViewport/ModelViewportViewModel.h"

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
            ModelHierarchyViewModel{ _context->GetApi<IModelApi>(), _context->GetApi<IPreviewApi>() }
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