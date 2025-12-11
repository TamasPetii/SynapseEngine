#include "FileSystemWindow.h"
#include <algorithm>
#include <sstream>

std::vector<std::string> SplitPath(const std::string& str, char delimiter) {
    std::vector<std::string> tokens;
    std::string token;
    std::istringstream tokenStream(str);
    while (std::getline(tokenStream, token, delimiter)) {
        if (!token.empty()) tokens.push_back(token);
    }
    return tokens;
}

FileSystemWindow::FileSystemWindow(std::shared_ptr<IFileSystem> fsProvider, const std::string& startPath)
    : m_FileSystem(fsProvider), m_CurrentPath(startPath)
{
    m_ThumbnailSize = 90.0f;
}

void FileSystemWindow::Render(std::shared_ptr<Registry> registry, std::shared_ptr<ResourceManager> resourceManager, std::set<VkDescriptorSet>& textureSet, uint32_t frameIndex)
{
    if (!m_Icons.Folder.first)
    {
        auto imageManager = resourceManager->GetImageManager();
        auto vulkanManager = resourceManager->GetVulkanManager();
        auto sampler = vulkanManager->GetSampler("Linear")->Value();

        auto loadIcon = [&](const std::string& path) -> std::pair<std::shared_ptr<ImageTexture>, VkDescriptorSet> {
            auto img = imageManager->LoadImage(path, ImageLoadMode::Sync, false);
            VkDescriptorSet ds = VK_NULL_HANDLE;
            if (img) {
                ds = ImGui_ImplVulkan_AddTexture(sampler, img->GetImage()->GetImageView("Default"), VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
            }
            return { img, ds };
            };

        m_Icons.Folder = loadIcon("../Assets/Engine/Icons/Folder.png");
        m_Icons.File = loadIcon("../Assets/Engine/Icons/File.png");
        m_Icons.Image = loadIcon("../Assets/Engine/Icons/Image.png");
        m_Icons.Code = loadIcon("../Assets/Engine/Icons/Code.png");
        m_Icons.Model = loadIcon("../Assets/Engine/Icons/Model.png");
        m_Icons.Sound = loadIcon("../Assets/Engine/Icons/Sound.png");
    }

    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
    ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.11f, 0.11f, 0.11f, 1.00f));

    if (ImGui::Begin(ICON_FA_FOLDER_OPEN "  Content Browser", nullptr, ImGuiWindowFlags_NoScrollbar))
    {
        float topBarHeight = 40.0f;

        ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.15f, 0.15f, 0.15f, 1.00f));
        ImGui::BeginChild("##TopBar", ImVec2(0, topBarHeight), false, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);
        {
            RenderBreadCrumbs();
        }
        ImGui::EndChild();
        ImGui::PopStyleColor();

        ImGui::BeginChild("##ContentArea", ImVec2(0, 0), false, ImGuiWindowFlags_AlwaysUseWindowPadding);
        {
            ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(8, 8));
            RenderContentArea();
            ImGui::PopStyleVar();
        }
        ImGui::EndChild();
    }

    ImGui::End();
    ImGui::PopStyleColor();
    ImGui::PopStyleVar();
}

void FileSystemWindow::RenderBreadCrumbs()
{
    ImGui::SetCursorPos(ImVec2(8, 8));

    if (ImGui::Button(std::string(ICON_FA_ARROW_UP).c_str())) {
        std::string parent = m_FileSystem->GetParentPath(m_CurrentPath);
        if (!parent.empty()) ChangeDirectory(parent);
    }

    ImGui::SameLine();

    ImGui::Dummy(ImVec2(1.0f, ImGui::GetFrameHeight()));
    ImGui::SameLine(0, 0);

    std::string pathStr = m_CurrentPath;
    std::replace(pathStr.begin(), pathStr.end(), '\\', '/');
    auto parts = SplitPath(pathStr, '/');

    std::string currentBuildPath = "";

    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 0, 0, 0));

    for (size_t i = 0; i < parts.size(); ++i) {
        currentBuildPath += parts[i];

        if (ImGui::Button(parts[i].c_str())) {
            ChangeDirectory(currentBuildPath);
        }

        if (i < parts.size() - 1) {
            ImGui::SameLine();
            ImGui::TextDisabled(ICON_FA_CHEVRON_RIGHT);
            ImGui::SameLine();
            currentBuildPath += "/";
        }
    }
    ImGui::PopStyleColor();

    float sliderWidth = 120.0f;
    float avail = ImGui::GetContentRegionAvail().x;
    if (avail > sliderWidth + 20) {
        ImGui::SameLine(ImGui::GetWindowWidth() - sliderWidth - 10);
        ImGui::SetNextItemWidth(sliderWidth);
        ImGui::SliderFloat("##Scale", &m_ThumbnailSize, 48.0f, 196.0f, "Zoom");
    }
}

void FileSystemWindow::RenderContentArea()
{
    if (!m_FileSystem) return;

    auto entries = m_FileSystem->GetEntries(m_CurrentPath);

    float panelWidth = ImGui::GetContentRegionAvail().x;
    float padding = 16.0f;
    float cellSize = m_ThumbnailSize + padding;
    int columnCount = (int)(panelWidth / cellSize);
    if (columnCount < 1) columnCount = 1;

    ImGui::Columns(columnCount, "FileSystemGrid", false);

    for (auto& entry : entries)
    {
        RenderFileCard(entry, m_ThumbnailSize);
        ImGui::NextColumn();
    }

    ImGui::Columns(1);
}

void FileSystemWindow::RenderFileCard(const FileEntry& entry, float cardSize)
{
    ImGui::PushID(entry.Path.c_str());

    bool isSelected = (m_SelectedPath == entry.Path);
    ImVec4 baseCol = isSelected ? ImVec4(0.26f, 0.59f, 0.98f, 0.4f) : ImVec4(0, 0, 0, 0);
    ImVec4 hoverCol = isSelected ? ImVec4(0.26f, 0.59f, 0.98f, 0.5f) : ImVec4(1.0f, 1.0f, 1.0f, 0.05f);

    ImGui::PushStyleColor(ImGuiCol_Button, baseCol);
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, hoverCol);
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.26f, 0.59f, 0.98f, 0.7f));
    ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 4.0f);

    float textHeight = ImGui::GetTextLineHeightWithSpacing() * 2;
    ImVec2 totalSize = ImVec2(cardSize, cardSize + textHeight);

    if (ImGui::Button("##CardButton", totalSize)) {
        m_SelectedPath = entry.Path;
    }

    auto iconPair = GetIconForEntry(entry);
    VkDescriptorSet iconDS = iconPair.second;

    if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_SourceAllowNullID))
    {
        std::string payloadType = "FILE_PATH";
        if (entry.Extension == ".obj" || entry.Extension == ".fbx" || entry.Extension == ".gltf")
            payloadType = "Model";
        else if (entry.Extension == ".png" || entry.Extension == ".jpg")
            payloadType = "Texture";

        ImGui::SetDragDropPayload(payloadType.c_str(), entry.Path.c_str(), entry.Path.size() + 1);

        if (iconDS != VK_NULL_HANDLE) {
            ImGui::Image((ImTextureID)iconDS, ImVec2(48, 48));
        }
        ImGui::Text("%s", entry.Name.c_str());
        ImGui::EndDragDropSource();
    }

    if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left))
    {
        if (entry.IsDirectory) {
            ChangeDirectory(entry.Path);
        }
    }

    ImVec2 itemMin = ImGui::GetItemRectMin();
    ImGui::SetItemAllowOverlap();

    if (iconDS != VK_NULL_HANDLE)
    {
        ImGui::SetCursorScreenPos(itemMin);
        ImGui::Image((ImTextureID)iconDS, ImVec2(cardSize, cardSize));
    }

    std::string displayName = entry.Name;
    float textWidth = ImGui::CalcTextSize(displayName.c_str()).x;
    float textIndent = (cardSize - textWidth) * 0.5f;
    if (textIndent < 0) textIndent = 0;

    ImGui::SetCursorScreenPos(ImVec2(itemMin.x + textIndent, itemMin.y + cardSize));
    ImGui::PushTextWrapPos(ImGui::GetCursorPos().x + (cardSize - textIndent));
    ImGui::TextUnformatted(displayName.c_str());
    ImGui::PopTextWrapPos();

    ImGui::PopStyleVar();
    ImGui::PopStyleColor(3);
    ImGui::PopID();
}

std::pair<std::shared_ptr<ImageTexture>, VkDescriptorSet> FileSystemWindow::GetIconForEntry(const FileEntry& entry)
{
    if (entry.IsDirectory) return m_Icons.Folder;

    if (entry.Extension == ".png" || entry.Extension == ".jpg" || entry.Extension == ".tga") return m_Icons.Image;
    if (entry.Extension == ".cpp" || entry.Extension == ".h" || entry.Extension == ".cs" || entry.Extension == ".shader") return m_Icons.Code;
    if (entry.Extension == ".obj" || entry.Extension == ".fbx" || entry.Extension == ".gltf") return m_Icons.Model;
    if (entry.Extension == ".mp3" || entry.Extension == ".wav") return m_Icons.Sound;

    return m_Icons.File;
}

void FileSystemWindow::ChangeDirectory(const std::string& newPath)
{
    m_CurrentPath = newPath;
    m_SelectedPath = "";
}