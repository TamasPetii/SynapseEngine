#include "ContentBrowserView.h"
#include "Editor/Manager/EditorIcons.h"
#include <imgui.h>
#include <sstream>
#include <algorithm>

namespace Syn {

    ContentBrowserView::ContentBrowserView(IIconManager* iconManager)
        : _iconManager(iconManager) {}

    void ContentBrowserView::Draw(ContentBrowserViewModel& vm) {
        const ContentBrowserState& state = vm.GetState();

        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
        ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.11f, 0.11f, 0.11f, 1.00f));

        if (ImGui::Begin(SYN_ICON_FOLDER_OPEN " Content Browser", nullptr, ImGuiWindowFlags_NoScrollbar)) {
            RenderTopBar(vm, state);
            RenderContentArea(vm, state);
        }

        ImGui::End();
        ImGui::PopStyleColor();
        ImGui::PopStyleVar();
    }

    void ContentBrowserView::RenderTopBar(ContentBrowserViewModel& vm, const ContentBrowserState& state) {
        float topBarHeight = 40.0f;
        ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.15f, 0.15f, 0.15f, 1.00f));

        if (ImGui::BeginChild("##TopBar", ImVec2(0, topBarHeight), false, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse)) {
            ImGui::SetCursorPos(ImVec2(8, 8));

            if (ImGui::Button(SYN_ICON_ARROW_UP)) {
                std::string parentPath = GetParentDirectory(state.currentPath);
                if (!parentPath.empty()) {
                    vm.Dispatch(ChangeDirectoryIntent{ parentPath });
                }
            }

            ImGui::SameLine();
            ImGui::Dummy(ImVec2(1.0f, ImGui::GetFrameHeight()));
            ImGui::SameLine(0, 0);

            RenderBreadCrumbs(vm, state.currentPath);

            float sliderWidth = 120.0f;
            float avail = ImGui::GetContentRegionAvail().x;
            if (avail > sliderWidth + 20) {
                ImGui::SameLine(ImGui::GetWindowWidth() - sliderWidth - 10);
                ImGui::SetNextItemWidth(sliderWidth);

                float currentScale = state.thumbnailSize;
                if (ImGui::SliderFloat("##Scale", &currentScale, 48.0f, 196.0f, "Zoom")) {
                    vm.Dispatch(SetThumbnailSizeIntent{ currentScale });
                }
            }
        }
        ImGui::EndChild();
        ImGui::PopStyleColor();
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
                ImGui::SameLine();
                ImGui::TextDisabled(SYN_ICON_CHEVRON_RIGHT);
                ImGui::SameLine();
                currentBuildPath += "/";
            }
        }
        ImGui::PopStyleColor();
    }

    void ContentBrowserView::RenderContentArea(ContentBrowserViewModel& vm, const ContentBrowserState& state) {
        ImGui::BeginChild("##ContentArea", ImVec2(0, 0), false, ImGuiWindowFlags_AlwaysUseWindowPadding);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(8, 8));

        float panelWidth = ImGui::GetContentRegionAvail().x;
        float padding = 16.0f;
        float cellSize = state.thumbnailSize + padding;
        int columnCount = std::max(1, static_cast<int>(panelWidth / cellSize));

        ImGui::Columns(columnCount, "FileSystemGrid", false);

        for (const auto& entry : state.currentEntries) {
            RenderFileCard(vm, state, entry);
            ImGui::NextColumn();
        }

        ImGui::Columns(1);
        ImGui::PopStyleVar();
        ImGui::EndChild();
    }

    void ContentBrowserView::RenderFileCard(ContentBrowserViewModel& vm, const ContentBrowserState& state, const FileEntry& entry) {
        ImGui::PushID(entry.path.c_str());

        bool isSelected = (state.selectedPath == entry.path);
        ImVec4 baseCol = isSelected ? ImVec4(0.26f, 0.59f, 0.98f, 0.4f) : ImVec4(0, 0, 0, 0);
        ImVec4 hoverCol = isSelected ? ImVec4(0.26f, 0.59f, 0.98f, 0.5f) : ImVec4(1.0f, 1.0f, 1.0f, 0.05f);

        ImGui::PushStyleColor(ImGuiCol_Button, baseCol);
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, hoverCol);
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.26f, 0.59f, 0.98f, 0.7f));
        ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 4.0f);

        float cardSize = state.thumbnailSize;
        float textHeight = ImGui::GetTextLineHeightWithSpacing() * 2;
        ImVec2 totalSize = ImVec2(cardSize, cardSize + textHeight);

        if (ImGui::Button("##CardButton", totalSize)) {
            vm.Dispatch(SelectEntryIntent{ entry.path });
        }

        if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left)) {
            if (entry.isDirectory) {
                vm.Dispatch(ChangeDirectoryIntent{ entry.path });
            }
        }

        if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_SourceAllowNullID)) {
            std::string payloadType = GetPayloadType(entry.extension);
            ImGui::SetDragDropPayload(payloadType.c_str(), entry.path.c_str(), entry.path.size() + 1);
            ImGui::TextUnformatted(entry.name.c_str());
            ImGui::EndDragDropSource();
        }

        ImVec2 itemMin = ImGui::GetItemRectMin();
        ImGui::SetItemAllowOverlap();

        ImTextureID iconID = GetIconForEntry(entry);
        if (iconID) {
            ImGui::SetCursorScreenPos(itemMin);
            ImGui::Image(iconID, ImVec2(cardSize, cardSize));
        }

        float textWidth = ImGui::CalcTextSize(entry.name.c_str()).x;
        float textIndent = std::max(0.0f, (cardSize - textWidth) * 0.5f);

        ImGui::SetCursorScreenPos(ImVec2(itemMin.x + textIndent, itemMin.y + cardSize));
        ImGui::PushTextWrapPos(ImGui::GetCursorPos().x + (cardSize - textIndent));
        ImGui::TextUnformatted(entry.name.c_str());
        ImGui::PopTextWrapPos();

        ImGui::PopStyleVar();
        ImGui::PopStyleColor(3);
        ImGui::PopID();
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