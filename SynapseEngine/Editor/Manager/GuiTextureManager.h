#pragma once
#include <vulkan/vulkan.h>
#include <imgui.h>
#include <unordered_map>
#include "EditorCore/Types/TextureHandle.h"

namespace Syn {
    class GuiTextureManager {
    public:
        static GuiTextureManager& Get() { static GuiTextureManager instance; return instance; }

        TextureHandle RegisterTexture(VkImageView imageView, VkSampler sampler);
        ImTextureID GetImGuiTextureID(TextureHandle handle);
        void RemoveTexture(TextureHandle handle);
        void Cleanup();
    private:
        GuiTextureManager() = default;
        TextureHandle _nextHandle = 1;
        std::unordered_map<TextureHandle, VkDescriptorSet> _textures;
    };
}