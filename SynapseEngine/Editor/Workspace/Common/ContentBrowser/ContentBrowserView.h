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

#pragma once
#include "Editor/Workspace/IView.h"
#include "EditorCore/ViewModels/Common/ContentBrowser/ContentBrowserViewModel.h"
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