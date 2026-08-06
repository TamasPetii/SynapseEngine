#include "AudioWorkspace.h"
#include "Editor/Manager/EditorIcons.h"

#include "Editor/Workspace/Common/ContentBrowser/ContentBrowserView.h"
#include "EditorCore/ViewModels/Common/ContentBrowser/ContentBrowserViewModel.h"

#include "View/AudioHierarchy/AudioHierarchyView.h"
#include "EditorCore/ViewModels/AudioWorkspace/AudioHierarchy/AudioHierarchyViewModel.h"

namespace Syn {

    AudioWorkspace::AudioWorkspace(EditorContext* context, IconManager* iconManager, const std::string& assetPath)
        : _context(context), _iconManager(iconManager), _assetPath(assetPath) {}

    void AudioWorkspace::OnActivate() {
    }

    void AudioWorkspace::Initialize() {
        using ContentBrowserWin = EditorWindow<ContentBrowserView, ContentBrowserViewModel>;

        AddWindow<ContentBrowserWin>(
            ContentBrowserView{ _iconManager, SYN_ICON_FOLDER_OPEN " Content Browser###Content_Audio" },
            ContentBrowserViewModel{ _context->GetApi<IFileSystemApi>(), _assetPath }
        );

        using AudioHierarchyWin = EditorWindow<AudioHierarchyView, AudioHierarchyViewModel>;
        AddWindow<AudioHierarchyWin>(
            AudioHierarchyView{},
            AudioHierarchyViewModel{
                _context->GetApi<IAudioApi>(),
                _context->GetApi<IPreviewApi>()
            }
        );
    }
}