#include "GuiTextureManager.h"
#include "Editor/Backends/imgui_impl_vulkan.h"
#include "Engine/ServiceLocator.h"
#include "Engine/FrameContext.h"

namespace Syn {
    TextureHandle GuiTextureManager::RegisterTexture(VkImageView imageView, VkSampler sampler) {
        VkDescriptorSet ds = ImGui_ImplVulkan_AddTexture(sampler, imageView, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
        TextureHandle handle = _nextHandle++;
        _textures[handle] = ds;
        return handle;
    }

    ImTextureID GuiTextureManager::GetImGuiTextureID(TextureHandle handle) {
        auto it = _textures.find(handle);
        if (it != _textures.end()) {
            return (ImTextureID)it->second;
        }
        return InvalidTextureHandle;
    }

    void GuiTextureManager::MarkForDeletion(TextureHandle handle) {
        auto it = _textures.find(handle);
        if (it != _textures.end()) {
            if (_deletionQueues.empty()) {
                uint32_t framesInFlight = ServiceLocator::GetFrameContext()->framesInFlight;
                _deletionQueues.resize(framesInFlight);
            }

            _deletionQueues[_currentFrameIndex].push_back(it->second);
            _textures.erase(it);
        }
    }

    void GuiTextureManager::SetCurrentFrame(uint32_t currentFrameIndex) {
        _currentFrameIndex = currentFrameIndex;

        if (_deletionQueues.empty()) {
            uint32_t framesInFlight = ServiceLocator::GetFrameContext()->framesInFlight;
            _deletionQueues.resize(framesInFlight);
        }
    }

    void GuiTextureManager::FlushQueue(uint32_t frameIndex) {
        if (_deletionQueues.empty() || frameIndex >= _deletionQueues.size()) return;

        for (VkDescriptorSet ds : _deletionQueues[frameIndex]) {
            ImGui_ImplVulkan_RemoveTexture(ds);
        }

        _deletionQueues[frameIndex].clear();
    }

    void GuiTextureManager::Cleanup() {
        for (auto& pair : _textures) {
            ImGui_ImplVulkan_RemoveTexture(pair.second);
        }
        _textures.clear();

        for (auto& queue : _deletionQueues) {
            for (VkDescriptorSet ds : queue) {
                ImGui_ImplVulkan_RemoveTexture(ds);
            }
            queue.clear();
        }
    }
}