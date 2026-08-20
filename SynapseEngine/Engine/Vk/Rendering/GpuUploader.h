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
#include "Engine/SynApi.h"
#include "Engine/Vk/Command/CommandPool.h"
#include "Engine/Vk/Command/CommandBuffer.h"
#include "Engine/Vk/Synchronization/Fence.h"
#include "Engine/Vk/Buffer/BufferUtils.h"
#include "Engine/Vk/Image/ImageUtils.h"
#include <vulkan/vulkan.h>
#include <vector>
#include <functional>
#include <mutex>
#include <memory>

namespace Syn::Vk
{
    class GpuUploader;

    enum class GpuQueueType {
        Transfer,
        Graphics,
        Compute,
        Video
    };

    struct SYN_API GpuUploadRequest {
        std::function<void(VkCommandBuffer, GpuUploader*)> uploadCallback;
        std::function<void()> onFinished;
        GpuQueueType queueType = GpuQueueType::Transfer;
    };

    struct PendingAcquires {
        std::vector<Vk::BufferBarrierInfo> buffers;
        std::vector<Vk::ImageBarrierInfo> images;
    };

    struct SYN_API BufferTransferData {
        VkBuffer buffer;
        VkDeviceSize size;
    };

    struct SYN_API ImageTransferData {
        VkImage image;
        VkImageAspectFlags aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        uint32_t mipLevels = 1;
        VkImageLayout oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
        VkImageLayout newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    };

    class SYN_API GpuUploader {
    public:
        GpuUploader();
        ~GpuUploader() = default;

        void Submit(GpuUploadRequest request);
        void Enqueue(GpuUploadRequest request);
        void UploadSync(GpuUploadRequest request);
        void ProcessUploads();

        void RegisterBufferTransfer(const BufferTransferData& data);
        void RegisterImageTransfer(const ImageTransferData& data);
        void RecordAcquireBarriers(VkCommandBuffer cmd);
    private:
        struct ActiveBatch {
            std::shared_ptr<Vk::Fence> fence;
            std::unique_ptr<Vk::CommandBuffer> cmd;
            std::vector<std::function<void()>> callbacks;
            std::vector<Vk::BufferBarrierInfo> acquireBuffers;
            std::vector<Vk::ImageBarrierInfo> acquireImages;
        };

        void ProcessQueue(std::vector<GpuUploadRequest>& requests, Vk::ThreadSafeQueue* queue, Vk::CommandPool* pool);
        PendingAcquires InsertReleaseBarriers(VkCommandBuffer cmd, Vk::ThreadSafeQueue* queue);

        std::vector<ActiveBatch> _activeBatches;
        std::vector<GpuUploadRequest> _transferRequests;
        std::vector<GpuUploadRequest> _graphicsRequests;
        std::vector<GpuUploadRequest> _videoRequests;
        std::vector<GpuUploadRequest> _computeRequests;

        Vk::ThreadSafeQueue* _transferQueue = nullptr;
        Vk::ThreadSafeQueue* _graphicsQueue = nullptr;
        Vk::ThreadSafeQueue* _videoQueue = nullptr;
        Vk::ThreadSafeQueue* _computeQueue = nullptr;

        std::unique_ptr<Vk::CommandPool> _transferPool;
        std::unique_ptr<Vk::CommandPool> _graphicsPool;
        std::unique_ptr<Vk::CommandPool> _videoPool;
        std::unique_ptr<Vk::CommandPool> _computePool;

        std::mutex _mutex;

        std::mutex _transferMutex;
        std::vector<BufferTransferData> _currentBatchBuffers;
        std::vector<ImageTransferData> _currentBatchImages;

        std::vector<Vk::BufferBarrierInfo> _readyAcquireBuffers;
        std::vector<Vk::ImageBarrierInfo> _readyAcquireImages;
    };
}