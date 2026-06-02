#pragma once
#include "Editor/View/IView.h"
#include "EditorCore/ViewModels/ContentBrowser/ContentBrowserViewModel.h"
#include "Editor/Manager/IIconManager.h"

namespace Syn {
    class ContentBrowserView : public IView<ContentBrowserViewModel> {
    public:
        explicit ContentBrowserView(IIconManager* iconManager);
        ~ContentBrowserView() override = default;

        void Draw(ContentBrowserViewModel& vm) override;
    private:
        void RenderTopBar(ContentBrowserViewModel& vm, const ContentBrowserState& state);
        void RenderBreadCrumbs(ContentBrowserViewModel& vm, const std::string& currentPath);
        void RenderContentArea(ContentBrowserViewModel& vm, const ContentBrowserState& state);
        void RenderFileCard(ContentBrowserViewModel& vm, const ContentBrowserState& state, const FileEntry& entry);

        ImTextureID GetIconForEntry(const FileEntry& entry) const;
        std::string GetPayloadType(const std::string& extension) const;
        std::string GetParentDirectory(const std::string& path) const;
        std::vector<std::string> SplitPath(const std::string& str, char delimiter) const;
    private:
        IIconManager* _iconManager = nullptr;
    };
}