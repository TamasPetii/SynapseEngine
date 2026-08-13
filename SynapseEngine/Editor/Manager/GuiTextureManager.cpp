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

#include "GuiTextureManager.h"
#include <imgui_impl_vulkan.h>
#include "Engine/ServiceLocator.h"
#include "Engine/FrameContext.h"

namespace Syn {
    GuiTextureManager::~GuiTextureManager() {
        Cleanup();
    }

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

    void GuiTextureManager::SetCurrentFrame(uint32_t currentFrameIndex) {
        _currentFrameIndex = currentFrameIndex;

        if (_deletionQueues.empty()) {
            uint32_t framesInFlight = ServiceLocator::Get<FrameContext>()->framesInFlight;
            _deletionQueues.resize(framesInFlight);
        }
    }

    void GuiTextureManager::MarkForDeletion(TextureHandle handle) {
        auto it = _textures.find(handle);
        if (it != _textures.end()) {
            _stagingQueue.push_back(it->second);
            _textures.erase(it);
        }
    }

    void GuiTextureManager::FlushQueue(uint32_t frameIndex) {
        if (_deletionQueues.empty()) {
            auto ctx = ServiceLocator::Get<FrameContext>();
            uint32_t framesInFlight = ctx ? ctx->framesInFlight : 3;
            _deletionQueues.resize(framesInFlight);
        }

        for (VkDescriptorSet ds : _deletionQueues[frameIndex]) {
            ImGui_ImplVulkan_RemoveTexture(ds);
        }

        _deletionQueues[frameIndex].clear();

        if (!_stagingQueue.empty()) {
            _deletionQueues[frameIndex] = _stagingQueue;
            _stagingQueue.clear();
        }
    }

    void GuiTextureManager::Cleanup() {
        for (auto& pair : _textures) {
            ImGui_ImplVulkan_RemoveTexture(pair.second);
        }
        _textures.clear();

        for (VkDescriptorSet ds : _stagingQueue) {
            ImGui_ImplVulkan_RemoveTexture(ds);
        }
        _stagingQueue.clear();

        for (auto& queue : _deletionQueues) {
            for (VkDescriptorSet ds : queue) {
                ImGui_ImplVulkan_RemoveTexture(ds);
            }
            queue.clear();
        }
    }
}