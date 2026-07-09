#include "TextureWorkspace.h"
#include "Editor/Manager/EditorIcons.h"

#include "Editor/View/ContentBrowser/ContentBrowserView.h"
#include "EditorCore/ViewModels/ContentBrowser/ContentBrowserViewModel.h"

#include "Editor/View/TextureHierarchy/TextureHierarchyView.h"
#include "EditorCore/ViewModels/TextureHierarchy/TextureHierarchyViewModel.h"

#include "Editor/View/TextureProperties/TexturePropertiesView.h"
#include "EditorCore/ViewModels/TextureProperties/TexturePropertiesViewModel.h"

#include "Editor/View/TextureGraph/TextureGraphView.h"
#include "EditorCore/ViewModels/TextureGraph/TextureGraphViewModel.h"

namespace Syn {

    TextureWorkspace::TextureWorkspace(EditorContext* context, IconManager* iconManager, const std::string& assetPath)
        : _context(context), _iconManager(iconManager), _assetPath(assetPath) {}

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