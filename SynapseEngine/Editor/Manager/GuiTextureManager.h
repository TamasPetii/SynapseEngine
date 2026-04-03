#pragma once
#include <vulkan/vulkan.h>
#include <imgui.h>
#include <vector>
#include <unordered_map>
#include "EditorCore/Types/TextureHandle.h"

namespace Syn {
    class GuiTextureManager {
    public:
        static GuiTextureManager& Get() { static GuiTextureManager instance; return instance; }

        TextureHandle RegisterTexture(VkImageView imageView, VkSampler sampler);
        ImTextureID GetImGuiTextureID(TextureHandle handle);

        void MarkForDeletion(TextureHandle handle);
        void SetCurrentFrame(uint32_t currentFrameIndex);
        void FlushQueue(uint32_t frameIndex);
        void Cleanup();

    private:
        GuiTextureManager() = default;
        TextureHandle _nextHandle = 1;
        uint32_t _currentFrameIndex = 0;

        std::vector<VkDescriptorSet> _stagingQueue;
        std::vector<std::vector<VkDescriptorSet>> _deletionQueues;
        std::unordered_map<TextureHandle, VkDescriptorSet> _textures;
    };
}