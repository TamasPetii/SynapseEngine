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

#include "MaterialWorkspace.h"
#include "Editor/Manager/EditorIcons.h"

#include "Editor/Workspace/Common/ContentBrowser/ContentBrowserView.h"
#include "EditorCore/ViewModels/Common/ContentBrowser/ContentBrowserViewModel.h"

#include "View/MaterialGraph/MaterialGraphView.h"
#include "EditorCore/ViewModels/MaterialWorkspace/MaterialGraph/MaterialGraphViewModel.h"

#include "View/MaterialHierarchy/MaterialHierarchyView.h"
#include "EditorCore/ViewModels/MaterialWorkspace/MaterialHierarchy/MaterialHierarchyViewModel.h"

#include "View/MaterialProperties/MaterialPropertiesView.h"
#include "EditorCore/ViewModels/MaterialWorkspace/MaterialProperties/MaterialPropertiesViewModel.h"

#include "View/MaterialViewport/MaterialViewportView.h"
#include "EditorCore/ViewModels/MaterialWorkspace/MaterialViewport/MaterialViewportViewModel.h"

#include "Engine/Scene/SceneNames.h"

namespace Syn {

    MaterialWorkspace::MaterialWorkspace(EditorContext* context, IconManager* iconManager, const std::string& assetPath)
        : _context(context), _iconManager(iconManager), _assetPath(assetPath) {}

    void MaterialWorkspace::OnActivate() {
        if (_context && _context->GetApi<ISceneApi>()) {
            _context->GetApi<ISceneApi>()->ActivateScene(SceneNames::MaterialPreview);
        }
    }

    void MaterialWorkspace::Initialize() {
        using ContentBrowserWin = EditorWindow<ContentBrowserView, ContentBrowserViewModel>;
        AddWindow<ContentBrowserWin>(
            ContentBrowserView{ _iconManager, SYN_ICON_FOLDER_OPEN " Content Browser###Content_Material" },
            ContentBrowserViewModel{ _context->GetApi<IFileSystemApi>(), _assetPath }
        );

        using MaterialHierarchyWin = EditorWindow<MaterialHierarchyView, MaterialHierarchyViewModel>;
        AddWindow<MaterialHierarchyWin>(
            MaterialHierarchyView{},
            MaterialHierarchyViewModel{ _context->GetApi<IMaterialApi>(), _context->GetApi<IPreviewApi>() }
        );

        using MaterialGraphWin = EditorWindow<MaterialGraphView, MaterialGraphViewModel>;
        AddWindow<MaterialGraphWin>(
            MaterialGraphView{},
            MaterialGraphViewModel{ _context->GetApi<IMaterialApi>(), _context->GetApi<ITextureApi>() }
        );

        using MaterialPropertiesWin = EditorWindow<MaterialPropertiesView, MaterialPropertiesViewModel>;
        AddWindow<MaterialPropertiesWin>(
            MaterialPropertiesView{},
            MaterialPropertiesViewModel{ _context->GetApi<IMaterialApi>(), _context->GetApi<ITextureApi>() }
        );

        using MaterialViewportWin = EditorWindow<MaterialViewportView, MaterialViewportViewModel>;
        AddWindow<MaterialViewportWin>(
            MaterialViewportView{},
            MaterialViewportViewModel{ _context->GetApi<IRenderApi>() }
        );
    }

}