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
#include <vulkan/vulkan.h>
#include <imgui.h>
#include <vector>
#include <unordered_map>
#include "EditorCore/Types/TextureHandle.h"

namespace Syn {
    class GuiTextureManager {
    public:
        GuiTextureManager() = default;
        ~GuiTextureManager();

        TextureHandle RegisterTexture(VkImageView imageView, VkSampler sampler);
        ImTextureID GetImGuiTextureID(TextureHandle handle);

        void MarkForDeletion(TextureHandle handle);
        void SetCurrentFrame(uint32_t currentFrameIndex);
        void FlushQueue(uint32_t frameIndex);
        void Cleanup();
    private:
        TextureHandle _nextHandle = 1;
        uint32_t _currentFrameIndex = 0;

        std::vector<VkDescriptorSet> _stagingQueue;
        std::vector<std::vector<VkDescriptorSet>> _deletionQueues;
        std::unordered_map<TextureHandle, VkDescriptorSet> _textures;
    };
}