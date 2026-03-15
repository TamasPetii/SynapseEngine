#include "GpuUploader.h"
#include "Engine/ServiceLocator.h"
#include "Engine/Vk/Context.h"
#include "Engine/Vk/Core/Device.h"

namespace Syn::Vk {

    GpuUploader::GpuUploader() {
        auto device = ServiceLocator::GetVkContext()->GetDevice();
        _transferQueue = device->GetTransferQueue();
        _graphicsQueue = device->GetGraphicsQueue();

        if (!_transferQueue) _transferQueue = _graphicsQueue;

        _transferPool = std::make_unique<Vk::CommandPool>(_transferQueue, VK_COMMAND_POOL_CREATE_TRANSIENT_BIT);
        _graphicsPool = std::make_unique<Vk::CommandPool>(_graphicsQueue, VK_COMMAND_POOL_CREATE_TRANSIENT_BIT);
    }

    void GpuUploader::Enqueue(GpuUploadRequest request) {
        std::lock_guard<std::mutex> lock(_mutex);
        if (request.needsGraphics) {
            _graphicsRequests.push_back(std::move(request));
        }
        else {
            _transferRequests.push_back(std::move(request));
        }
    }

    void GpuUploader::ProcessUploads() {
        std::lock_guard<std::mutex> lock(_mutex);

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
        std::lock_guard<std::mutex> lock(_mutex);

        auto device = ServiceLocator::GetVkContext()->GetDevice();
        Vk::ThreadSafeQueue* queue = request.needsGraphics ? _graphicsQueue : _transferQueue;
        Vk::CommandPool* pool = request.needsGraphics ? _graphicsPool.get() : _transferPool.get();

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