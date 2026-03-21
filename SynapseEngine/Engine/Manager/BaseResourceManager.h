#pragma once
#include "Engine/SynApi.h"
#include <vector>
#include <string>
#include <unordered_map>
#include <future>
#include <mutex>
#include <span>
#include <optional>

#include "Engine/Vk/Synchronization/Fence.h"
#include "Engine/Vk/Buffer/Buffer.h"
#include "Engine/Vk/Command/CommandBuffer.h"
#include "Engine/ServiceLocator.h"
#include "Engine/Vk/Core/ThreadSafeQueue.h"
#include "Engine/Vk/Rendering/GpuUploader.h"

namespace Syn {

    enum class ResourceState {
        LoadingCPU,
        UploadingGPU,
        Ready,
        Failed
    };

    struct ResourceSyncControl {
        std::mutex mutex;
        std::condition_variable cv;
        ResourceState stateCopy = ResourceState::LoadingCPU;
    };

    template <typename TResource>
    struct ResourceEntry {
        ResourceState state = ResourceState::LoadingCPU;
        std::string path;
        bool isSyncLoad = false;

        std::shared_ptr<TResource> resource;
        std::unique_ptr<Vk::Buffer> stagingBuffer;
        std::future<std::shared_ptr<TResource>> cpuFuture;
        std::shared_ptr<ResourceSyncControl> sync = std::make_shared<ResourceSyncControl>();
    };

    template <typename TResource>
    class BaseResourceManager {
    protected:
        using EntryType = ResourceEntry<TResource>;
    public:
        struct ResourceSnapshot {
            std::shared_ptr<TResource> resource;
            ResourceState state;
        };

        virtual ~BaseResourceManager() = default;

        void Update();
        void WaitForResource(uint32_t id) const;
        void SetResourceState(uint32_t id, ResourceState newState);

        size_t GetResourceCount() const;
        ResourceState GetEntryState(uint32_t id) const;
        uint32_t GetVersion() const;
        uint32_t GetResourceIndex(const std::string & name) const;
        std::shared_ptr<TResource> GetResource(uint32_t id) const;
        std::shared_ptr<TResource> GetResource(const std::string & name) const;
        std::vector<ResourceSnapshot> GetResourceSnapshot() const;
    protected:
        uint32_t InternalLoadAsync(const std::string & key, std::function<std::shared_ptr<TResource>()> task);
        uint32_t InternalLoadSync(const std::string & key, std::function<std::shared_ptr<TResource>()> task);
        uint32_t InternalLoad(const std::string & key, std::function<std::shared_ptr<TResource>()> task, bool isAsync);
        std::shared_ptr<TResource> GetResource(uint32_t id, bool internalCall) const;
    protected:
        void SubmitGpuRequest(const EntryType & entry, Vk::GpuUploadRequest && request);
        virtual void StartGpuUpload(EntryType & entry) = 0;
        virtual void FinalizeResource(EntryType & entry) = 0;
    protected:
        std::atomic<uint32_t> _version;
        std::vector<EntryType> _entries;
        std::unordered_map<std::string, uint32_t> _pathToId;
        mutable std::recursive_mutex _mutex;
    };

    template <typename TResource>
    void BaseResourceManager<TResource>::Update() {
        std::lock_guard lock(_mutex);

        for (auto& entry : _entries) {
            if (entry.state == ResourceState::LoadingCPU) {
                if (entry.cpuFuture.valid() && entry.cpuFuture.wait_for(std::chrono::seconds(0)) == std::future_status::ready) {
                    auto result = entry.cpuFuture.get();
                    if (result != nullptr) {
                        entry.resource = result;
                        entry.state = ResourceState::UploadingGPU;
                        StartGpuUpload(entry);
                    }
                    else {
                        entry.state = ResourceState::Failed;
                    }
                }
            }
        }
    }

    template <typename TResource>
    uint32_t BaseResourceManager<TResource>::GetResourceIndex(const std::string& name) const
    {
        std::lock_guard lock(_mutex);
        if (_pathToId.find(name) == _pathToId.end()) return UINT32_MAX;
        return _pathToId.at(name);
    }

    template <typename TResource>
    std::shared_ptr<TResource> BaseResourceManager<TResource>::GetResource(uint32_t id) const {
        std::lock_guard lock(_mutex);
        return GetResource(id, true);
    }

    template <typename TResource>
    std::shared_ptr<TResource> BaseResourceManager<TResource>::GetResource(const std::string& name) const {
        std::lock_guard lock(_mutex);
        if (_pathToId.find(name) == _pathToId.end()) return nullptr;
        return GetResource(_pathToId.at(name), true);
    }

    template <typename TResource>
    uint32_t BaseResourceManager<TResource>::InternalLoad(const std::string& key, std::function<std::shared_ptr<TResource>()> task, bool isAsync) {
        std::lock_guard lock(_mutex);

        if (_pathToId.contains(key)) {
            return _pathToId[key];
        }

        uint32_t newId = static_cast<uint32_t>(_entries.size());
        _pathToId[key] = newId;

        EntryType newEntry{};
        newEntry.path = key;
        newEntry.state = ResourceState::LoadingCPU;
        newEntry.isSyncLoad = !isAsync;

        if (isAsync) {
            auto executor = ServiceLocator::GetTaskExecutor();
            newEntry.cpuFuture = executor->async(std::move(task));
            _entries.push_back(std::move(newEntry));
        }
        else {
            newEntry.resource = task();

            if (newEntry.resource != nullptr) {
                newEntry.state = ResourceState::UploadingGPU;
                _entries.push_back(std::move(newEntry));
                StartGpuUpload(_entries.back());
            }
            else {
                newEntry.state = ResourceState::Failed;
                _entries.push_back(std::move(newEntry));
            }
        }

        return newId;
    }

    template <typename TResource>
    uint32_t BaseResourceManager<TResource>::InternalLoadAsync(const std::string& key, std::function<std::shared_ptr<TResource>()> task) {
        return InternalLoad(key, std::move(task), true);
    }

    template <typename TResource>
    uint32_t BaseResourceManager<TResource>::InternalLoadSync(const std::string& key, std::function<std::shared_ptr<TResource>()> task) {
        return InternalLoad(key, std::move(task), false);
    }

    template <typename TResource>
    std::shared_ptr<TResource> BaseResourceManager<TResource>::GetResource(uint32_t id, bool) const {
        if (id >= _entries.size()) return nullptr;
        if (_entries[id].state == ResourceState::Ready) return _entries[id].resource;
        return nullptr;
    }

    template <typename TResource>
    size_t BaseResourceManager<TResource>::GetResourceCount() const {
        std::lock_guard lock(_mutex);
        return _entries.size();
    }

    template <typename TResource>
    ResourceState BaseResourceManager<TResource>::GetEntryState(uint32_t id) const
    {
        std::lock_guard lock(_mutex);
        return _entries[id].state;
    }

    template <typename TResource>
    void BaseResourceManager<TResource>::SubmitGpuRequest(const EntryType& entry, Vk::GpuUploadRequest&& request) {
        auto uploader = ServiceLocator::GetGpuUploader();
        if (entry.isSyncLoad) {
            uploader->UploadSync(std::move(request));
        }
        else {
            uploader->Enqueue(std::move(request));
        }
    }

    template <typename TResource>
    std::vector<typename BaseResourceManager<TResource>::ResourceSnapshot> BaseResourceManager<TResource>::GetResourceSnapshot() const {
        std::lock_guard lock(_mutex);
        std::vector<ResourceSnapshot> snapshot;
        snapshot.reserve(_entries.size());

        for (const auto& entry : _entries) {
            snapshot.push_back({ entry.resource, entry.state });
        }
        return snapshot;
    }

    template <typename TResource>
    uint32_t BaseResourceManager<TResource>::GetVersion() const {
        return _version.load(std::memory_order_acquire);
    }

    template <typename TResource>
    void BaseResourceManager<TResource>::WaitForResource(uint32_t id) const {
        std::shared_ptr<ResourceSyncControl> syncBlock;

        {
            std::lock_guard lock(_mutex);
            if (id >= _entries.size()) return;
            syncBlock = _entries[id].sync;
        }

        std::unique_lock<std::mutex> lock(syncBlock->mutex);
        syncBlock->cv.wait(lock, [&syncBlock]() {
            return syncBlock->stateCopy == ResourceState::Ready || syncBlock->stateCopy == ResourceState::Failed;
            });
    }

    template <typename TResource>
    void BaseResourceManager<TResource>::SetResourceState(uint32_t id, ResourceState newState) {
        std::shared_ptr<ResourceSyncControl> syncBlock;

        {
            std::lock_guard lock(_mutex);
            if (id >= _entries.size()) return;

            _entries[id].state = newState;
            syncBlock = _entries[id].sync;
            syncBlock->stateCopy = newState;
        }

        syncBlock->cv.notify_all();
    }
}