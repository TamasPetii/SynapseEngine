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

#include "GpuUploader.h"
#include "Engine/ServiceLocator.h"
#include "Engine/Vk/Context.h"
#include "Engine/Vk/Core/Device.h"

namespace Syn::Vk {

    GpuUploader::GpuUploader() {
        auto device = ServiceLocator::Get<Vk::Context>()->GetDevice();
        _transferQueue = device->GetTransferQueue();
        _graphicsQueue = device->GetGraphicsQueue();
        _videoQueue = device->GetVideoDecodeQueue();
        _computeQueue = device->GetComputeQueue();

        if (!_transferQueue) _transferQueue = _graphicsQueue;

        _transferPool = std::make_unique<Vk::CommandPool>(_transferQueue, VK_COMMAND_POOL_CREATE_TRANSIENT_BIT);
        _graphicsPool = std::make_unique<Vk::CommandPool>(_graphicsQueue, VK_COMMAND_POOL_CREATE_TRANSIENT_BIT);

        if (_videoQueue) {
            _videoPool = std::make_unique<Vk::CommandPool>(_videoQueue, VK_COMMAND_POOL_CREATE_TRANSIENT_BIT);
        }

        if (_computeQueue) {
            _computePool = std::make_unique<Vk::CommandPool>(_computeQueue, VK_COMMAND_POOL_CREATE_TRANSIENT_BIT);
        }
    }

    void GpuUploader::Enqueue(GpuUploadRequest request) {
        std::lock_guard lock(_mutex);

        if (request.queueType == GpuQueueType::Video && _videoQueue) {
            _videoRequests.push_back(std::move(request));
        }
        else if (request.queueType == GpuQueueType::Compute && _computeQueue) {
            _computeRequests.push_back(std::move(request));
        }
        else if (request.queueType == GpuQueueType::Graphics && _graphicsQueue) {
            _graphicsRequests.push_back(std::move(request));
        }
        else {
            _transferRequests.push_back(std::move(request));
        }
    }

    void GpuUploader::Submit(GpuUploadRequest request) {
        Enqueue(std::move(request));
    }

    void GpuUploader::ProcessUploads() {
        std::lock_guard lock(_mutex);

        auto it = _activeBatches.begin();

        while (it != _activeBatches.end()) {
            if (it->fence->IsSignaled())
            {
                if (!it->acquireBuffers.empty() || !it->acquireImages.empty()) {
                    std::lock_guard transferLock(_transferMutex);
                    _readyAcquireBuffers.insert(_readyAcquireBuffers.end(), it->acquireBuffers.begin(), it->acquireBuffers.end());
                    _readyAcquireImages.insert(_readyAcquireImages.end(), it->acquireImages.begin(), it->acquireImages.end());
                }

                for (auto& cb : it->callbacks) if (cb) cb();
                it = _activeBatches.erase(it);
            }
            else {
                ++it;
            }
        }

        if (!_transferRequests.empty()) {
            ProcessQueue(_transferRequests, _transferQueue, _transferPool.get());
        }

        if (!_graphicsRequests.empty() && _graphicsQueue) {
            ProcessQueue(_graphicsRequests, _graphicsQueue, _graphicsPool.get());
        }

        if (!_computeRequests.empty() && _computeQueue) {
            ProcessQueue(_computeRequests, _computeQueue, _computePool.get());
        }

        if (!_videoRequests.empty() && _videoQueue) {
            ProcessQueue(_videoRequests, _videoQueue, _videoPool.get());
        }
    }

    void GpuUploader::ProcessQueue(std::vector<GpuUploadRequest>& requests, Vk::ThreadSafeQueue* queue, Vk::CommandPool* pool) {
        auto cmd = pool->AllocateBuffer(VK_COMMAND_BUFFER_LEVEL_PRIMARY);
        auto fence = std::make_shared<Vk::Fence>(false);
        std::vector<std::function<void()>> callbacks;

        cmd->Begin(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);
        for (auto& req : requests) {
            req.uploadCallback(cmd->Handle(), this);
            callbacks.push_back(std::move(req.onFinished));
        }

        auto pendingAcquires = InsertReleaseBarriers(cmd->Handle(), queue);

        cmd->End();

        VkCommandBufferSubmitInfo cmdSubmitInfo{ VK_STRUCTURE_TYPE_COMMAND_BUFFER_SUBMIT_INFO };
        cmdSubmitInfo.commandBuffer = cmd->Handle();

        VkSubmitInfo2 submitInfo{ VK_STRUCTURE_TYPE_SUBMIT_INFO_2 };
        submitInfo.commandBufferInfoCount = 1;
        submitInfo.pCommandBufferInfos = &cmdSubmitInfo;

        queue->Submit(&submitInfo, fence->Handle());

        _activeBatches.push_back({
            fence,
            std::move(cmd),
            std::move(callbacks),
            std::move(pendingAcquires.buffers),
            std::move(pendingAcquires.images)
            });

        requests.clear();
    }

    void GpuUploader::UploadSync(GpuUploadRequest request) {
        std::lock_guard lock(_mutex);

        auto device = ServiceLocator::Get<Vk::Context>()->GetDevice();

        Vk::ThreadSafeQueue* queue = _transferQueue;
        Vk::CommandPool* pool = _transferPool.get();

        if (request.queueType == GpuQueueType::Video && _videoQueue) {
            queue = _videoQueue;
            pool = _videoPool.get();
        }
        else if (request.queueType == GpuQueueType::Compute && _computeQueue) {
            queue = _computeQueue;
            pool = _computePool.get();
        }
        else if (request.queueType == GpuQueueType::Graphics && _graphicsQueue) {
            queue = _graphicsQueue;
            pool = _graphicsPool.get();
        }

        auto cmd = pool->AllocateBuffer(VK_COMMAND_BUFFER_LEVEL_PRIMARY);
        auto fence = std::make_shared<Vk::Fence>(false);

        cmd->Begin(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);
        request.uploadCallback(cmd->Handle(), this);
        auto pendingAcquires = InsertReleaseBarriers(cmd->Handle(), queue);
        cmd->End();

        VkCommandBufferSubmitInfo cmdSubmitInfo{ VK_STRUCTURE_TYPE_COMMAND_BUFFER_SUBMIT_INFO };
        cmdSubmitInfo.commandBuffer = cmd->Handle();

        VkSubmitInfo2 submitInfo{ VK_STRUCTURE_TYPE_SUBMIT_INFO_2 };
        submitInfo.commandBufferInfoCount = 1;
        submitInfo.pCommandBufferInfos = &cmdSubmitInfo;

        queue->Submit(&submitInfo, fence->Handle());

        VkFence vkFence = fence->Handle();
        vkWaitForFences(device->Handle(), 1, &vkFence, VK_TRUE, UINT64_MAX);

        if (!pendingAcquires.buffers.empty() || !pendingAcquires.images.empty()) {
            std::lock_guard transferLock(_transferMutex);
            _readyAcquireBuffers.insert(_readyAcquireBuffers.end(), pendingAcquires.buffers.begin(), pendingAcquires.buffers.end());
            _readyAcquireImages.insert(_readyAcquireImages.end(), pendingAcquires.images.begin(), pendingAcquires.images.end());
        }

        if (request.onFinished) {
            request.onFinished();
        }
    }

    void GpuUploader::RegisterBufferTransfer(const BufferTransferData& data) {
        std::lock_guard lock(_transferMutex);
        _currentBatchBuffers.push_back(data);
    }

    void GpuUploader::RegisterImageTransfer(const ImageTransferData& data) {
        std::lock_guard lock(_transferMutex);
        _currentBatchImages.push_back(data);
    }

    PendingAcquires GpuUploader::InsertReleaseBarriers(VkCommandBuffer cmd, Vk::ThreadSafeQueue* queue) {
        uint32_t srcQueueFamily = queue->GetFamilyIndex();
        uint32_t graphicsQueueFamily = _graphicsQueue->GetFamilyIndex();
        bool needsOwnershipTransfer = (srcQueueFamily != graphicsQueueFamily);

        std::vector<Vk::BufferBarrierInfo> releaseBuffers;
        std::vector<Vk::ImageBarrierInfo> releaseImages;
        PendingAcquires pending;

        {
            std::lock_guard transferLock(_transferMutex);

            releaseBuffers.reserve(_currentBatchBuffers.size());
            for (const auto& buf : _currentBatchBuffers) {
                Vk::BufferBarrierInfo b{};
                b.buffer = buf.buffer;
                b.size = buf.size;
                b.srcStage = VK_PIPELINE_STAGE_2_TRANSFER_BIT;
                b.srcAccess = VK_ACCESS_2_TRANSFER_WRITE_BIT;
                b.dstStage = needsOwnershipTransfer ? 0 : VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT;
                b.dstAccess = needsOwnershipTransfer ? 0 : VK_ACCESS_2_MEMORY_READ_BIT;
                b.srcQueueFamilyIndex = needsOwnershipTransfer ? srcQueueFamily : VK_QUEUE_FAMILY_IGNORED;
                b.dstQueueFamilyIndex = needsOwnershipTransfer ? graphicsQueueFamily : VK_QUEUE_FAMILY_IGNORED;
                releaseBuffers.push_back(b);

                if (needsOwnershipTransfer) {
                    b.srcStage = 0;
                    b.srcAccess = 0;
                    b.dstStage = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT;
                    b.dstAccess = VK_ACCESS_2_MEMORY_READ_BIT;
                    pending.buffers.push_back(b);
                }
            }

            releaseImages.reserve(_currentBatchImages.size());
            for (const auto& img : _currentBatchImages) {
                Vk::ImageBarrierInfo b{};
                b.image = img.image;
                b.aspectMask = img.aspectMask;
                b.levelCount = img.mipLevels;
                b.oldLayout = img.oldLayout;
                b.newLayout = img.newLayout;
                b.srcStage = VK_PIPELINE_STAGE_2_TRANSFER_BIT;
                b.srcAccess = VK_ACCESS_2_TRANSFER_WRITE_BIT;
                b.dstStage = needsOwnershipTransfer ? 0 : VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT;
                b.dstAccess = needsOwnershipTransfer ? 0 : VK_ACCESS_2_MEMORY_READ_BIT;
                b.srcQueueFamilyIndex = needsOwnershipTransfer ? srcQueueFamily : VK_QUEUE_FAMILY_IGNORED;
                b.dstQueueFamilyIndex = needsOwnershipTransfer ? graphicsQueueFamily : VK_QUEUE_FAMILY_IGNORED;
                releaseImages.push_back(b);

                if (needsOwnershipTransfer) {
                    b.srcStage = 0;
                    b.srcAccess = 0;
                    b.dstStage = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT;
                    b.dstAccess = VK_ACCESS_2_MEMORY_READ_BIT;
                    pending.images.push_back(b);
                }
            }

            _currentBatchBuffers.clear();
            _currentBatchImages.clear();
        }

        if (!releaseBuffers.empty()) Vk::BufferUtils::InsertBatchedBarriers(cmd, releaseBuffers);
        if (!releaseImages.empty()) Vk::ImageUtils::InsertBatchedBarriers(cmd, releaseImages);

        return pending;
    }

    void GpuUploader::RecordAcquireBarriers(VkCommandBuffer cmd) {
        std::vector<Vk::BufferBarrierInfo> buffersToAcquire;
        std::vector<Vk::ImageBarrierInfo> imagesToAcquire;

        {
            std::lock_guard lock(_transferMutex);
            if (_readyAcquireBuffers.empty() && _readyAcquireImages.empty()) return;

            buffersToAcquire = std::move(_readyAcquireBuffers);
            imagesToAcquire = std::move(_readyAcquireImages);
        }

        if (!buffersToAcquire.empty()) {
            Vk::BufferUtils::InsertBatchedBarriers(cmd, buffersToAcquire);
        }

        if (!imagesToAcquire.empty()) {
            Vk::ImageUtils::InsertBatchedBarriers(cmd, imagesToAcquire);
        }
    }
}