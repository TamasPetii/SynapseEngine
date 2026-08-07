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