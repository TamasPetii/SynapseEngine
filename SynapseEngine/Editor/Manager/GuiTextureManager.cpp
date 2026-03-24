#include "GuiTextureManager.h"
#include "Editor/Backends/imgui_impl_vulkan.h"

namespace Syn {
    TextureHandle GuiTextureManager::RegisterTexture(VkImageView imageView, VkSampler sampler) {
        VkDescriptorSet ds = ImGui_ImplVulkan_AddTexture(sampler, imageView, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
        TextureHandle handle = _nextHandle++;
        _textures[handle] = ds;
        return handle;
    }

    ImTextureID GuiTextureManager::GetImGuiTextureID(TextureHandle handle) {
        if (_textures.find(handle) != _textures.end()) {
            return (ImTextureID)_textures[handle];
        }
        return 0;
    }

    void GuiTextureManager::RemoveTexture(TextureHandle handle) {
        if (_textures.find(handle) != _textures.end()) {
            ImGui_ImplVulkan_RemoveTexture(_textures[handle]);
            _textures.erase(handle);
        }
    }

    void GuiTextureManager::Cleanup() {
        _textures.clear();
    }
}