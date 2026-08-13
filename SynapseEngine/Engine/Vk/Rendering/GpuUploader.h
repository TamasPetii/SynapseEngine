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
#include <vector>
#include <functional>
#include <mutex>
#include <memory>

namespace Syn::Vk
{
    struct SYN_API GpuUploadRequest {
        std::function<void(VkCommandBuffer)> uploadCallback;
        std::function<void()> onFinished;
        bool needsGraphics = false;
        bool needsVideo = false;
    };

    class SYN_API GpuUploader {
    public:
        GpuUploader();
        ~GpuUploader() = default;

        void Submit(GpuUploadRequest request);
        void Enqueue(GpuUploadRequest request);
        void UploadSync(GpuUploadRequest request);
        void ProcessUploads();
    private:
        struct ActiveBatch {
            std::shared_ptr<Vk::Fence> fence;
            std::unique_ptr<Vk::CommandBuffer> cmd;
            std::vector<std::function<void()>> callbacks;
        };

        void ProcessQueue(std::vector<GpuUploadRequest>& requests, Vk::ThreadSafeQueue* queue, Vk::CommandPool* pool);

        std::vector<ActiveBatch> _activeBatches;
        std::vector<GpuUploadRequest> _transferRequests;
        std::vector<GpuUploadRequest> _graphicsRequests;
        std::vector<GpuUploadRequest> _videoRequests;

        Vk::ThreadSafeQueue* _transferQueue = nullptr;
        Vk::ThreadSafeQueue* _graphicsQueue = nullptr;
        Vk::ThreadSafeQueue* _videoQueue = nullptr;

        std::unique_ptr<Vk::CommandPool> _transferPool;
        std::unique_ptr<Vk::CommandPool> _graphicsPool;
        std::unique_ptr<Vk::CommandPool> _videoPool;

        std::mutex _mutex;
    };
}