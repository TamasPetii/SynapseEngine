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

namespace Syn {

    enum class ResourceState {
        LoadingCPU,
        UploadingGPU,
        Ready,
        Failed
    };

    template <typename TResource>
    struct ResourceEntry {
        ResourceState state = ResourceState::LoadingCPU;
        std::string path;

        std::shared_ptr<TResource> resource;
        std::unique_ptr<Vk::Buffer> stagingBuffer;
        std::future<std::shared_ptr<TResource>> cpuFuture;
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

        std::vector<ResourceSnapshot> GetResourceSnapshot() const {
            std::lock_guard<std::mutex> lock(_mutex);
            std::vector<ResourceSnapshot> snapshot;
            snapshot.reserve(_entries.size());

            for (const auto& entry : _entries) {
                snapshot.push_back({ entry.resource, entry.state });
            }
            return snapshot;
        }

        virtual ~BaseResourceManager() = default;

        void Update();
        size_t GetResourceCount() const;
        ResourceState GetEntryState(uint32_t id) const;
        std::shared_ptr<TResource> GetResource(uint32_t id) const;
        std::shared_ptr<TResource> GetResource(const std::string& name) const;
        uint32_t GetVersion() const { return _version.load(std::memory_order_acquire); }
    protected:
        uint32_t InternalLoadAsync(const std::string& key, std::function<std::shared_ptr<TResource>()> task);
        std::shared_ptr<TResource> GetResource(uint32_t id, bool internalCall) const;
    protected:
        virtual void StartGpuUpload(EntryType& entry) = 0;
        virtual void FinalizeResource(EntryType& entry) = 0;
    protected:
        std::atomic<uint32_t> _version;
        std::vector<EntryType> _entries;
        std::unordered_map<std::string, uint32_t> _pathToId;
        mutable std::mutex _mutex;
    };

    template <typename TResource>
    void BaseResourceManager<TResource>::Update() {
        std::lock_guard<std::mutex> lock(_mutex);

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
    std::shared_ptr<TResource> BaseResourceManager<TResource>::GetResource(uint32_t id) const {
        std::lock_guard<std::mutex> lock(_mutex);
        return GetResource(id, true);
    }

    template <typename TResource>
    std::shared_ptr<TResource> BaseResourceManager<TResource>::GetResource(const std::string& name) const {
        std::lock_guard<std::mutex> lock(_mutex);
        if (_pathToId.find(name) == _pathToId.end()) return nullptr;
        return GetResource(_pathToId.at(name), true);
    }

    template <typename TResource>
    uint32_t BaseResourceManager<TResource>::InternalLoadAsync(const std::string& key, std::function<std::shared_ptr<TResource>()> task) {
        std::lock_guard<std::mutex> lock(_mutex);

        if (_pathToId.contains(key)) {
            return _pathToId[key];
        }

        uint32_t newId = static_cast<uint32_t>(_entries.size());
        _pathToId[key] = newId;

        EntryType newEntry{};
        newEntry.path = key;
        newEntry.state = ResourceState::LoadingCPU;

        auto executor = ServiceLocator::GetTaskExecutor();
        newEntry.cpuFuture = executor->async(std::move(task));

        _entries.push_back(std::move(newEntry));
        return newId;
    }

    template <typename TResource>
    std::shared_ptr<TResource> BaseResourceManager<TResource>::GetResource(uint32_t id, bool) const {
        if (id >= _entries.size()) return nullptr;
        if (_entries[id].state == ResourceState::Ready) return _entries[id].resource;
        return nullptr;
    }

    template <typename TResource>
    size_t BaseResourceManager<TResource>::GetResourceCount() const {
        std::lock_guard<std::mutex> lock(_mutex);
        return _entries.size();
    }

    template <typename TResource>
    ResourceState BaseResourceManager<TResource>::GetEntryState(uint32_t id) const
    {
        std::lock_guard<std::mutex> lock(_mutex);
        return _entries[id].state;
    }
}