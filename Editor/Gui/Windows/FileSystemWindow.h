#pragma once
#include "Window.h"
#include "../Utils/FileSystem.h"
#include <memory>
#include <string>
#include <utility>
#include "IconsFontAwesome5.h" 
#include "Engine/Utils/ImageTexture.h"
#include <vulkan/vulkan.h>

class FileSystemWindow : public Window {
public:
    FileSystemWindow(std::shared_ptr<IFileSystem> fsProvider, const std::string& startPath);
    ~FileSystemWindow() = default;

    virtual void Render(std::shared_ptr<Registry> registry, std::shared_ptr<ResourceManager> resourceManager, std::set<VkDescriptorSet>& textureSet, uint32_t frameIndex) override;

private:
    void RenderBreadCrumbs();
    void RenderContentArea();
    void RenderFileCard(const FileEntry& entry, float cardSize);

    std::pair<std::shared_ptr<ImageTexture>, VkDescriptorSet> GetIconForEntry(const FileEntry& entry);
    void ChangeDirectory(const std::string& newPath);

private:
    std::shared_ptr<IFileSystem> m_FileSystem;
    std::string m_CurrentPath;
    std::string m_SelectedPath;

    float m_ThumbnailSize = 96.0f;
    float m_Padding = 16.0f;

    struct Icons {
        std::pair<std::shared_ptr<ImageTexture>, VkDescriptorSet> Folder;
        std::pair<std::shared_ptr<ImageTexture>, VkDescriptorSet> File;
        std::pair<std::shared_ptr<ImageTexture>, VkDescriptorSet> Image;
        std::pair<std::shared_ptr<ImageTexture>, VkDescriptorSet> Code;
        std::pair<std::shared_ptr<ImageTexture>, VkDescriptorSet> Model;
        std::pair<std::shared_ptr<ImageTexture>, VkDescriptorSet> Sound;
    } m_Icons;
};