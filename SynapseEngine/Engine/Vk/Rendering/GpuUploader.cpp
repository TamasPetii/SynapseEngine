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

        if (!_transferQueue) _transferQueue = _graphicsQueue;

        _transferPool = std::make_unique<Vk::CommandPool>(_transferQueue, VK_COMMAND_POOL_CREATE_TRANSIENT_BIT);
        _graphicsPool = std::make_unique<Vk::CommandPool>(_graphicsQueue, VK_COMMAND_POOL_CREATE_TRANSIENT_BIT);

        if (_videoQueue) {
            _videoPool = std::make_unique<Vk::CommandPool>(_videoQueue, VK_COMMAND_POOL_CREATE_TRANSIENT_BIT);
        }
    }

    void GpuUploader::Enqueue(GpuUploadRequest request) {
        std::lock_guard lock(_mutex);

        if (request.needsVideo && _videoQueue) {
            _videoRequests.push_back(std::move(request));
        }
        else if (request.needsGraphics) {
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
            if (it->fence->IsSignaled()) {
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

        if (!_graphicsRequests.empty()) {
            ProcessQueue(_graphicsRequests, _graphicsQueue, _graphicsPool.get());
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
            req.uploadCallback(cmd->Handle());
            callbacks.push_back(std::move(req.onFinished));
        }
        cmd->End();

        VkCommandBufferSubmitInfo cmdSubmitInfo{ VK_STRUCTURE_TYPE_COMMAND_BUFFER_SUBMIT_INFO };
        cmdSubmitInfo.commandBuffer = cmd->Handle();

        VkSubmitInfo2 submitInfo{ VK_STRUCTURE_TYPE_SUBMIT_INFO_2 };
        submitInfo.commandBufferInfoCount = 1;
        submitInfo.pCommandBufferInfos = &cmdSubmitInfo;

        queue->Submit(&submitInfo, fence->Handle());

        _activeBatches.push_back({ fence, std::move(cmd), std::move(callbacks) });
        requests.clear();
    }

    void GpuUploader::UploadSync(GpuUploadRequest request) {
        std::lock_guard lock(_mutex);

        auto device = ServiceLocator::Get<Vk::Context>()->GetDevice();

        Vk::ThreadSafeQueue* queue = _transferQueue;
        Vk::CommandPool* pool = _transferPool.get();

        if (request.needsVideo && _videoQueue) {
            queue = _videoQueue;
            pool = _videoPool.get();
        }
        else if (request.needsGraphics) {
            queue = _graphicsQueue;
            pool = _graphicsPool.get();
        }

        auto cmd = pool->AllocateBuffer(VK_COMMAND_BUFFER_LEVEL_PRIMARY);
        auto fence = std::make_shared<Vk::Fence>(false);

        cmd->Begin(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);
        request.uploadCallback(cmd->Handle());
        cmd->End();

        VkCommandBufferSubmitInfo cmdSubmitInfo{ VK_STRUCTURE_TYPE_COMMAND_BUFFER_SUBMIT_INFO };
        cmdSubmitInfo.commandBuffer = cmd->Handle();

        VkSubmitInfo2 submitInfo{ VK_STRUCTURE_TYPE_SUBMIT_INFO_2 };
        submitInfo.commandBufferInfoCount = 1;
        submitInfo.pCommandBufferInfos = &cmdSubmitInfo;

        queue->Submit(&submitInfo, fence->Handle());

        VkFence vkFence = fence->Handle();
        vkWaitForFences(device->Handle(), 1, &vkFence, VK_TRUE, UINT64_MAX);

        if (request.onFinished) {
            request.onFinished();
        }
    }
}