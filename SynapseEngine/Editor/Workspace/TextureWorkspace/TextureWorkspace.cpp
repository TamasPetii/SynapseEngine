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

#include "TextureWorkspace.h"
#include "Editor/Manager/EditorIcons.h"

#include "Editor/Workspace/Common/ContentBrowser/ContentBrowserView.h"
#include "EditorCore/ViewModels/Common/ContentBrowser/ContentBrowserViewModel.h"

#include "View/TextureHierarchy/TextureHierarchyView.h"
#include "EditorCore/ViewModels/TextureWorkspace/TextureHierarchy/TextureHierarchyViewModel.h"

#include "View/TextureProperties/TexturePropertiesView.h"
#include "EditorCore/ViewModels/TextureWorkspace/TextureProperties/TexturePropertiesViewModel.h"

#include "View/TextureGraph/TextureGraphView.h"
#include "EditorCore/ViewModels/TextureWorkspace/TextureGraph/TextureGraphViewModel.h"

#include "Engine/Scene/SceneNames.h"

namespace Syn {

    TextureWorkspace::TextureWorkspace(EditorContext* context, IconManager* iconManager, const std::string& assetPath)
        : _context(context), _iconManager(iconManager), _assetPath(assetPath) {}

	void TextureWorkspace::OnActivate()
	{
		if (_context && _context->GetApi<ISceneApi>()) {
			_context->GetApi<ISceneApi>()->ActivateScene(SceneNames::Empty);
		}
	}

    void TextureWorkspace::Initialize()
    {
        using ContentBrowserWin = EditorWindow<ContentBrowserView, ContentBrowserViewModel>;
        AddWindow<ContentBrowserWin>(
            ContentBrowserView{ _iconManager, SYN_ICON_FOLDER_OPEN " Content Browser###Content_Texture" },
            ContentBrowserViewModel{ _context->GetApi<IFileSystemApi>(), _assetPath }
        );

        using TextureHierarchyWin = EditorWindow<TextureHierarchyView, TextureHierarchyViewModel>;
        AddWindow<TextureHierarchyWin>(
            TextureHierarchyView{},
            TextureHierarchyViewModel{ _context->GetApi<ITextureApi>() }
        );

        using TexturePropertiesWin = EditorWindow<TexturePropertiesView, TexturePropertiesViewModel>;
        AddWindow<TexturePropertiesWin>(
            TexturePropertiesView{},
            TexturePropertiesViewModel{ _context->GetApi<ITextureApi>() }
        );

        using TextureGraphWin = EditorWindow<TextureGraphView, TextureGraphViewModel>;
        AddWindow<TextureGraphWin>(
            TextureGraphView{},
            TextureGraphViewModel{ _context->GetApi<ITextureApi>() }
        );
    }
}