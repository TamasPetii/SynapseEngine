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
#include "../VkCommon.h"
#include "ImageConfig.h"

namespace Syn::Vk 
{
    class ImageFactory;

    class SYN_API Image {
    public:
        explicit Image(const ImageConfig& config);
        Image(VkImage existingHandle, const ImageConfig& config); //Swapchain wrapper!
        ~Image();

        Image(const Image&) = delete;
        Image& operator=(const Image&) = delete;
        Image(Image&&) = delete;
        Image& operator=(Image&&) = delete;

        VkImage Handle() const { return _handle; }
        VkFormat GetFormat() const { return _config.format; }
        VkExtent3D GetExtent() const { return { _config.width, _config.height, _config.depth }; }
        const ImageConfig& GetConfig() const { return _config; }
        VkImageView GetView(const std::string& name = "") const;
		VkImageLayout GetLayout() const { return _currentLayout; }
        void TransitionLayout(VkCommandBuffer cmd, VkImageLayout newLayout, VkPipelineStageFlags2 dstStage, VkAccessFlags2 dstAccess, bool discardContent = false);
        void OverrideInternalState(VkImageLayout layout, VkPipelineStageFlags2 stage, VkAccessFlags2 access);
    private:
        ImageConfig _config;

        VkImage _handle = VK_NULL_HANDLE;
        VmaAllocator _allocator = VK_NULL_HANDLE;
        VmaAllocation _allocation = VK_NULL_HANDLE;

        VkImageLayout _currentLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        VkPipelineStageFlags2 _currentStage = VK_PIPELINE_STAGE_2_NONE;
        VkAccessFlags2 _currentAccess = VK_ACCESS_2_NONE;

        std::unordered_map<std::string, VkImageView> _imageViews;

        friend class ImageFactory;
    };
}