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

namespace Syn::Vk {

    struct SYN_API BufferCopyInfo {
        VkBuffer srcBuffer = VK_NULL_HANDLE;
        VkBuffer dstBuffer = VK_NULL_HANDLE;
        VkDeviceSize size = 0;
        VkDeviceSize srcOffset = 0;
        VkDeviceSize dstOffset = 0;
    };

    struct SYN_API BufferToImageCopyInfo {
        VkBuffer srcBuffer = VK_NULL_HANDLE;
        VkImage dstImage = VK_NULL_HANDLE;

        uint32_t width = 0;
        uint32_t height = 0;
        uint32_t depth = 1;

        VkDeviceSize bufferOffset = 0;

        VkOffset3D imageOffset = { 0, 0, 0 };

        uint32_t mipLevel = 0;
        uint32_t baseArrayLayer = 0;
        uint32_t layerCount = 1;
        VkImageAspectFlags aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        VkImageLayout dstImageLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
    };

    struct SYN_API BufferBarrierInfo {
        VkBuffer buffer = VK_NULL_HANDLE;
        VkDeviceSize size = VK_WHOLE_SIZE;
        VkDeviceSize offset = 0;

        VkPipelineStageFlags2 srcStage = VK_PIPELINE_STAGE_2_NONE;
        VkAccessFlags2 srcAccess = VK_ACCESS_2_NONE;
        VkPipelineStageFlags2 dstStage = VK_PIPELINE_STAGE_2_NONE;
        VkAccessFlags2 dstAccess = VK_ACCESS_2_NONE;

        uint32_t srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        uint32_t dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    };

    struct SYN_API GlobalBarrierInfo {
        VkPipelineStageFlags2 srcStage = VK_PIPELINE_STAGE_2_NONE;
        VkAccessFlags2 srcAccess = VK_ACCESS_2_NONE;
        VkPipelineStageFlags2 dstStage = VK_PIPELINE_STAGE_2_NONE;
        VkAccessFlags2 dstAccess = VK_ACCESS_2_NONE;
    };

    struct SYN_API BufferFillInfo {
        VkBuffer buffer = VK_NULL_HANDLE;
        VkDeviceSize offset = 0;
        VkDeviceSize size = VK_WHOLE_SIZE;
        uint32_t data = 0;
    };

    struct SYN_API BufferUpdateInfo {
        VkBuffer buffer = VK_NULL_HANDLE;
        VkDeviceSize offset = 0;
        VkDeviceSize size = 0;
        const void* pData = nullptr;
    };

    class SYN_API BufferUtils {
    public:
        static void CopyBuffer(VkCommandBuffer cmd, const BufferCopyInfo& info);
        static void CopyBufferToImage(VkCommandBuffer cmd, const BufferToImageCopyInfo& info);
        static void InsertBarrier(VkCommandBuffer cmd, const BufferBarrierInfo& info);
        static void InsertGlobalBarrier(VkCommandBuffer cmd, const GlobalBarrierInfo& info);
        static void FillBuffer(VkCommandBuffer cmd, const BufferFillInfo& info);
        static void UpdateBuffer(VkCommandBuffer cmd, const BufferUpdateInfo& info);
    };
}