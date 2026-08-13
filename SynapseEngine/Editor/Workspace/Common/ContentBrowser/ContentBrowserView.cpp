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

#include "ContentBrowserView.h"
#include "Editor/Manager/EditorIcons.h"
#include "Editor/Widgets/CardWidget.h"
#include "Editor/Widgets/ItemCardWidget.h"
#include "Editor/Widgets/ItemCardContainerWidget.h"
#include <imgui.h>
#include <imgui_internal.h>
#include <sstream>
#include <algorithm>

namespace Syn {

    ContentBrowserView::ContentBrowserView(IIconManager* iconManager, const std::string& windowTitle)
        : _iconManager(iconManager), _windowTitle(windowTitle) {}

    void ContentBrowserView::Draw(ContentBrowserViewModel& vm) {
        const ContentBrowserState& state = vm.GetState();

        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(8, 8));

        ImGuiWindowFlags windowFlags = ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse;

        if (ImGui::Begin(_windowTitle.c_str(), nullptr, windowFlags)) {

            auto getCardState = [this](const char* name) -> bool& {
                std::string key(name);
                if (_cardStates.find(key) == _cardStates.end()) _cardStates[key] = true;
                return _cardStates[key];
                };

            RenderTopBar(vm, state);

            ImGui::Spacing();

            float mainContentBottomY = ImGui::GetWindowPos().y + ImGui::GetWindowContentRegionMax().y;
            float currentY = ImGui::GetCursorScreenPos().y;
            float panelHeight = mainContentBottomY - currentY - 8.0f;
            if (panelHeight < 150.0f) panelHeight = 150.0f;

            ImGui::BeginChild("LeftPanelContainer", ImVec2(_leftPanelWidth, panelHeight), false, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);

            constexpr const char* CardFoldersTitle = "Folders";
            if (Syn::UI::BeginCard(CardFoldersTitle, SYN_ICON_FOLDER, getCardState(CardFoldersTitle))) {

                ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.08f, 0.08f, 0.08f, 0.6f));
                ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, 8.0f);
                ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(8, 8));

                float treeHeight = mainContentBottomY - ImGui::GetCursorScreenPos().y - 12.0f;
                if (treeHeight < 50.0f) treeHeight = 50.0f;

                ImGui::BeginChild("FolderTreeScroll", ImVec2(0, treeHeight), ImGuiChildFlags_Borders | ImGuiChildFlags_AlwaysUseWindowPadding, 0);
                RenderFolderTree(vm, state);
                ImGui::EndChild();

                ImGui::PopStyleVar(2);
                ImGui::PopStyleColor();
            }
            Syn::UI::EndCard();
            ImGui::EndChild();

            ImGui::SameLine(0, 0);

            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 0, 0, 0));
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.3f, 0.3f, 0.3f, 0.5f));
            ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.4f, 0.4f, 0.4f, 0.8f));

            ImGui::Button("##Splitter", ImVec2(6.0f, panelHeight));
            if (ImGui::IsItemHovered()) ImGui::SetMouseCursor(ImGuiMouseCursor_ResizeEW);
            if (ImGui::IsItemActive()) {
                _leftPanelWidth += ImGui::GetIO().MouseDelta.x;
                _leftPanelWidth = std::clamp(_leftPanelWidth, 150.0f, 600.0f);
            }

            ImGui::PopStyleColor(3);

            ImGui::SameLine(0, 0);

            ImGui::BeginChild("RightPanelContainer", ImVec2(0, panelHeight), false, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);

            constexpr const char* CardFilesTitle = "Content";
            if (Syn::UI::BeginCard(CardFilesTitle, SYN_ICON_FILE, getCardState(CardFilesTitle))) {

                ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.08f, 0.08f, 0.08f, 0.6f));
                ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, 8.0f);
                ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(8, 8));

                float gridHeight = mainContentBottomY - ImGui::GetCursorScreenPos().y - 12.0f;
                if (gridHeight < 50.0f) gridHeight = 50.0f;

                ImGui::BeginChild("ContentGridScroll", ImVec2(0, gridHeight), ImGuiChildFlags_Borders | ImGuiChildFlags_AlwaysUseWindowPadding, 0);
                RenderContentArea(vm, state);
                ImGui::EndChild();

                ImGui::PopStyleVar(2);
                ImGui::PopStyleColor();
            }
            Syn::UI::EndCard();
            ImGui::EndChild();
        }

        ImGui::End();
        ImGui::PopStyleVar();
    }

    void ContentBrowserView::RenderTopBar(ContentBrowserViewModel& vm, const ContentBrowserState& state) {
        ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(6, 6));

        float barHeight = ImGui::GetFrameHeight();
        ImGui::BeginChild("TopBar", ImVec2(0, barHeight), false, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);

        if (ImGui::Button(SYN_ICON_ARROW_UP)) {
            std::string parentPath = GetParentDirectory(state.currentPath);
            if (!parentPath.empty()) {
                vm.Dispatch(ChangeDirectoryIntent{ parentPath });
            }
        }
        if (ImGui::IsItemHovered()) ImGui::SetTooltip("Up to parent directory");

        ImGui::SameLine();
        ImGui::Dummy(ImVec2(8.0f, 0.0f));
        ImGui::SameLine();

        RenderBreadCrumbs(vm, state.currentPath);

        float sliderWidth = 120.0f;
        float avail = ImGui::GetContentRegionAvail().x;
        if (avail > sliderWidth + 20) {
            ImGui::SameLine(ImGui::GetWindowWidth() - sliderWidth - 8.0f);
            ImGui::SetNextItemWidth(sliderWidth);

            float currentScale = state.thumbnailSize;
            if (ImGui::SliderFloat("##Scale", &currentScale, 48.0f, 196.0f, " %.0f")) {
                vm.Dispatch(SetThumbnailSizeIntent{ currentScale });
            }
        }

        ImGui::EndChild();
        ImGui::PopStyleVar();
    }

    void ContentBrowserView::RenderBreadCrumbs(ContentBrowserViewModel& vm, const std::string& currentPath) {
        std::string pathStr = currentPath;
        std::replace(pathStr.begin(), pathStr.end(), '\\', '/');
        auto parts = SplitPath(pathStr, '/');

        std::string currentBuildPath = "";

        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 0, 0, 0));

        for (size_t i = 0; i < parts.size(); ++i) {
            currentBuildPath += parts[i];

            ImGui::PushID(static_cast<int>(i));

            if (ImGui::Button(parts[i].c_str())) {
                vm.Dispatch(ChangeDirectoryIntent{ currentBuildPath });
            }

            ImGui::PopID();

            if (i < parts.size() - 1) {
                ImGui::SameLine(0, 4.0f);
                ImGui::TextDisabled(SYN_ICON_CHEVRON_RIGHT);
                ImGui::SameLine(0, 4.0f);
                currentBuildPath += "/";
            }
        }
        ImGui::PopStyleColor();
    }

    void ContentBrowserView::RenderFolderTree(ContentBrowserViewModel& vm, const ContentBrowserState& state) {
        std::string pathStr = state.currentPath;
        std::replace(pathStr.begin(), pathStr.end(), '\\', '/');
        auto parts = SplitPath(pathStr, '/');

        if (parts.empty()) return;

        std::string buildPath = "";
        int depth = 0;

        for (size_t i = 0; i < parts.size(); ++i) {
            buildPath += parts[i];

            ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_SpanFullWidth | ImGuiTreeNodeFlags_DefaultOpen;

            if (i == parts.size() - 1) {
                flags |= ImGuiTreeNodeFlags_Selected;
            }

            std::string label = std::string(SYN_ICON_FOLDER_OPEN) + " " + parts[i];
            bool isOpen = ImGui::TreeNodeEx(buildPath.c_str(), flags, "%s", label.c_str());

            if (ImGui::IsItemClicked() && !ImGui::IsItemToggledOpen()) {
                vm.Dispatch(ChangeDirectoryIntent{ buildPath });
            }

            buildPath += "/";

            if (!isOpen) {
                break;
            }
            depth++;
        }

        if (depth == parts.size()) {
            for (const auto& entry : state.currentEntries) {
                if (entry.isDirectory) {
                    ImGuiTreeNodeFlags leafFlags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_SpanFullWidth | ImGuiTreeNodeFlags_Leaf;
                    std::string label = std::string(SYN_ICON_FOLDER) + " " + entry.name;

                    ImGui::TreeNodeEx(entry.path.c_str(), leafFlags, "%s", label.c_str());

                    if (ImGui::IsItemClicked() && !ImGui::IsItemToggledOpen()) {
                        vm.Dispatch(ChangeDirectoryIntent{ entry.path });
                    }
                    ImGui::TreePop();
                }
            }
        }

        for (int i = 0; i < depth; ++i) {
            ImGui::TreePop();
        }
    }

    void ContentBrowserView::RenderContentArea(ContentBrowserViewModel& vm, const ContentBrowserState& state) {
        const auto entries = state.currentEntries;
        Syn::UI::ItemCardContainer("ContentGrid", (int)entries.size(), state.thumbnailSize,
            [&](int index) {
                const FileEntry& entry = entries[index];

                Syn::UI::ItemCardDesc desc;
                desc.id = entry.path.c_str();
                desc.title = entry.name.c_str();
                desc.texture = GetIconForEntry(entry);
                desc.selected = (state.selectedPath == entry.path);

                desc.events.onClick = [&vm, &entry] {
                    vm.Dispatch(SelectEntryIntent{ entry.path });
                    };

                desc.events.onDoubleClick = [&vm, &entry] {
                    if (entry.isDirectory) vm.Dispatch(ChangeDirectoryIntent{ entry.path });
                    };

                desc.events.onDragDropSource = [this, &entry] {
                    std::string payloadType = GetPayloadType(entry.extension);
                    ImGui::SetDragDropPayload(payloadType.c_str(), entry.path.c_str(), entry.path.size() + 1);
                    ImGui::TextUnformatted(entry.name.c_str());
                    };

                Syn::UI::ItemCard(desc, state.thumbnailSize);
            });
    }

    ImTextureID ContentBrowserView::GetIconForEntry(const FileEntry& entry) const {
        if (!_iconManager) return 0;

        if (entry.isDirectory)
            return _iconManager->GetIconDescriptor(EditorIconType::Folder);

        if (entry.extension == ".png" || entry.extension == ".jpg" || entry.extension == ".tga")
            return _iconManager->GetIconDescriptor(EditorIconType::Image);

        if (entry.extension == ".cpp" || entry.extension == ".h" || entry.extension == ".shader")
            return _iconManager->GetIconDescriptor(EditorIconType::Code);

        if (entry.extension == ".obj" || entry.extension == ".fbx" || entry.extension == ".gltf")
            return _iconManager->GetIconDescriptor(EditorIconType::Model);

        if (entry.extension == ".mp3" || entry.extension == ".wav")
            return _iconManager->GetIconDescriptor(EditorIconType::Sound);

        return _iconManager->GetIconDescriptor(EditorIconType::File);
    }

    std::string ContentBrowserView::GetPayloadType(const std::string& extension) const {
        if (extension == ".obj" || extension == ".fbx" || extension == ".gltf") return "Model";
        if (extension == ".png" || extension == ".jpg" || extension == ".tga") return "Texture";
        return "FILE_PATH";
    }

    std::string ContentBrowserView::GetParentDirectory(const std::string& path) const {
        size_t lastSlash = path.find_last_of("/\\");
        if (lastSlash != std::string::npos) {
            return path.substr(0, lastSlash);
        }
        return "";
    }

    std::vector<std::string> ContentBrowserView::SplitPath(const std::string& str, char delimiter) const {
        std::vector<std::string> tokens;
        std::string token;
        std::istringstream tokenStream(str);
        while (std::getline(tokenStream, token, delimiter)) {
            if (!token.empty()) tokens.push_back(token);
        }
        return tokens;
    }
}