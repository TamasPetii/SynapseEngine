#pragma once
#include "Editor/View/IView.h"
#include "EditorCore/ViewModels/ContentBrowser/ContentBrowserViewModel.h"
#include "Editor/Manager/IIconManager.h"
#include <unordered_map>
#include <string>
#include <vector>

namespace Syn {
    class ContentBrowserView : public IView<ContentBrowserViewModel> {
    public:
        explicit ContentBrowserView(IIconManager* iconManager, const std::string& windowTitle);
        ~ContentBrowserView() override = default;

        void Draw(ContentBrowserViewModel& vm) override;
    private:
        void RenderTopBar(ContentBrowserViewModel& vm, const ContentBrowserState& state);
        void RenderBreadCrumbs(ContentBrowserViewModel& vm, const std::string& currentPath);

        void RenderFolderTree(ContentBrowserViewModel& vm, const ContentBrowserState& state);
        void RenderContentArea(ContentBrowserViewModel& vm, const ContentBrowserState& state);

        ImTextureID GetIconForEntry(const FileEntry& entry) const;
        std::string GetPayloadType(const std::string& extension) const;
        std::string GetParentDirectory(const std::string& path) const;
        std::vector<std::string> SplitPath(const std::string& str, char delimiter) const;

    private:
        IIconManager* _iconManager = nullptr;

        std::unordered_map<std::string, bool> _cardStates;
        float _leftPanelWidth = 250.0f;
        std::string _windowTitle;
    };
}